/* RTSPClient.cpp
 * Copyright (C) 2011 Société des arts technologiques (SAT)
 * Copyright (C) 2011 Tristan Matthews
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "RTSPClient.h"
#include <gst/gst.h>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "util/sigint.h"
#include "util/logWriter.h"
#include "gtk_utils/gtk_utils.h"
#include "gst/pipeline.h"
#include "gst/gstLinkable.h"

#if 0
gboolean RTSPClient::busCall(GstBus * /*bus*/, GstMessage *msg, void *user_data)
{
    RTSPClient *context = static_cast<RTSPClient*>(user_data);
    switch (GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_ERROR: 
            {
                GError *err;
                gchar *debug;
                gst_message_parse_error(msg, &err, &debug);
                LOG_WARNING("GOT ERROR " << err->message);
                g_error_free(err);
                g_free (debug);

                gutil::killMainLoop();

                return FALSE;
            }
        case GST_MESSAGE_WARNING:
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_warning(msg, &err, &debug);

                LOG_WARNING(gst_object_get_name(msg->src) << ":" << err->message);
                g_error_free(err);

                if (debug) {
                    LOG_DEBUG("Debug details: " << debug);
                    g_free(debug);
                }
                break;
            }

        case GST_MESSAGE_EOS: 
            {
                LOG_INFO("End-of-stream");
                gutil::killMainLoop();
                break;
            }

        case GST_MESSAGE_LATENCY:
            {
                // when pipeline latency is changed, this msg is posted on the bus. we then have
                // to explicitly tell the pipeline to recalculate its latency
                if (gst_bin_recalculate_latency (GST_BIN(context->pipeline_)) == TRUE)
                {
                    LOG_DEBUG("Reconfigured latency.");
                    context->latencySet_ = true;
                }
                else
                    LOG_DEBUG("Could not reconfigure latency.\n");
                break;
            }
        default:
            // Unhandled message
            break;
    }

    return TRUE;
}
#endif

gboolean
RTSPClient::timeout()
{
    if (signal_handlers::signalFlag())
    {
        gutil::killMainLoop();
        return FALSE;
    }
    else
        return TRUE;
}

namespace 
{
bool validPortRange(const std::string &ports)
{
    std::vector<std::string> strs;
    boost::split(strs, ports, boost::is_any_of("- "));
    if (strs.size() != 2)
        return false;
    int first = boost::lexical_cast<int>(strs[0]);
    int second = boost::lexical_cast<int>(strs[1]);
    // TODO Thu Mar 17 17:53:46 EDT 2011:tmatth 
    // this is the minimum if audio AND video are present, so we may want to change it
    // if the client knows in advance that it will not grab both streams
    static const int MINIMUM_PORT_RANGE = 5; // RTP=n, RTCP1=n+1, RTCP2=n+3
    if (first >= 1 and first <= 65535 and second >= 1 and second <= 65535)
        if ((second - first) >= MINIMUM_PORT_RANGE)
            return true;
    return false;
}
}

gboolean
RTSPClient::onNotifySource(GstElement *uridecodebin, GParamSpec * /*pspec*/, gpointer data)
{
    GstElement *src;
    RTSPClient *context = static_cast<RTSPClient*>(data);

    g_object_get (uridecodebin, "source", &src, NULL);

    /* set your properties (check for existance of
     * property first if you use different protocols
     * or sources) */
    LOG_DEBUG("Setting properties on rtspsrc");
    g_object_set (src, "latency", context->latency_, NULL);
    if (not context->portRange_.empty())
        g_object_set (src, "port-range", context->portRange_.c_str(), NULL);

    gst_object_unref (src); 
    return TRUE;
}

void
RTSPClient::onPadAdded(GstElement * /*uridecodebin*/, GstPad *pad, gpointer data)
{
    RTSPClient *context = static_cast<RTSPClient*>(data);
    GstCaps *caps = gst_pad_get_caps (pad);
    const gchar *mime = gst_structure_get_name (gst_caps_get_structure (caps, 0));
    if (g_strrstr (mime, "video"))
    {
        if (context->enableVideo_)
        {
            GstElement *videoQueue = context->pipeline_->findElementByName("video_queue");
            if (videoQueue == 0) 
            {
                LOG_WARNING("No element named video_queue, not linking");
                gst_caps_unref (caps);
                return;
            }
            GstPad *videoPad = gst_element_get_static_pad(videoQueue, "sink");
            if (GST_PAD_IS_LINKED(videoPad))
            {
                GstObject *parent = GST_OBJECT (GST_OBJECT_PARENT (videoPad));
                LOG_WARNING("Omitting link for pad " << GST_OBJECT_NAME(parent) << 
                        ":" << GST_OBJECT_NAME(videoPad) << " because it's already linked");
                gst_object_unref (GST_OBJECT (videoPad));
                gst_caps_unref (caps);
                return;
            }
            /* can it link to the videopad ? */
            GstCaps *videoCaps = gst_pad_get_caps (videoPad);
            GstCaps *res = gst_caps_intersect (caps, videoCaps);
            bool linked = false;
            if (res && !gst_caps_is_empty (res)) 
            {
                LOG_DEBUG("Found pad to link to videoqueue - plugging is now done");
                linked = gstlinkable::link_pads(pad, videoPad);
            }
            if (not linked) 
                LOG_WARNING("Could not link new pad to videoqueue");
            gst_caps_unref (videoCaps);
            gst_caps_unref (res);
            gst_object_unref (GST_OBJECT (videoPad));
            gst_caps_unref (caps);
        }
        else
        {
            LOG_WARNING("Got video stream even though we've disabled video");
            gst_caps_unref (caps);
        }
    }
    else if (g_strrstr (mime, "audio"))
    {
        if (context->enableAudio_)
        {
            GstElement *audioQueue = context->pipeline_->findElementByName("audio_queue");
            if (audioQueue == 0) 
            {
                LOG_WARNING("No element named audio_queue, not linking");
                gst_caps_unref (caps);
                return;
            }
            GstPad *audioPad = gst_element_get_static_pad(audioQueue, "sink");
            if (GST_PAD_IS_LINKED(audioPad))
            {
                GstObject *parent = GST_OBJECT (GST_OBJECT_PARENT (audioPad));
                LOG_WARNING("Omitting link for pad " << GST_OBJECT_NAME(parent) << 
                        ":" << GST_OBJECT_NAME(audioPad) << " because it's already linked");
                gst_object_unref (GST_OBJECT (audioPad));
                gst_caps_unref (caps);
                return;
            }
            /* can it link to the audiopad? */
            GstCaps *audioCaps = gst_pad_get_caps (audioPad);
            GstCaps *res = gst_caps_intersect (caps, audioCaps);
            bool linked = false;
            if (res && !gst_caps_is_empty (res)) 
            {
                LOG_DEBUG("Found pad to link to audioqueue - plugging is now done");
                linked = gstlinkable::link_pads(pad, audioPad);
            }
            if (not linked)
                LOG_WARNING("Could not link new pad to audioqueue");
            gst_caps_unref (audioCaps);
            gst_caps_unref (res);
            gst_object_unref (GST_OBJECT (audioPad));
        }
        else
        {
            LOG_WARNING("Got audio stream even though we've disabled audio");
            gst_caps_unref (caps);
        }
    }
    else
    {
        LOG_WARNING("Got unknown mimetype " << mime);
        gst_caps_unref (caps);
    }
}

static const int USEC_PER_MILLISEC = G_USEC_PER_SEC / 1000.0;

RTSPClient::RTSPClient(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio) :
    pipeline_(new Pipeline), 
    latencySet_(false), 
    portRange_(""),
    latency_(options["jitterbuffer"].as<int>()), 
    enableVideo_(enableVideo), 
    enableAudio_(enableAudio)
{
    using std::string;
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline_->makeVerbose();

    GstElement *uridecodebin = pipeline_->makeElement("uridecodebin", "decode");
    string uri("rtsp://" + options["address"].as<string>() + ":8554/test");
    g_object_set(uridecodebin, "uri", uri.c_str(), NULL);
    g_signal_connect(uridecodebin, "notify::source", G_CALLBACK(onNotifySource), this);
    g_signal_connect(uridecodebin, "pad-added", G_CALLBACK(onPadAdded), this);

    // get port range
    if (options.count("port-range"))
    {
        if (validPortRange(options["port-range"].as<std::string>()))
            portRange_ = options["port-range"].as<std::string>();
        else
            LOG_WARNING("Invalid port-range " << options["port-range"].as<std::string>() << ", ignoring.");
    }

    if (enableVideo_)
    {
        LOG_DEBUG("Video enabled");
        GstElement *queue = pipeline_->makeElement("queue", "video_queue");
        GstElement *colorspace = pipeline_->makeElement("ffmpegcolorspace", 0);
        GstElement *videosink = pipeline_->makeElement(options["videosink"].as<string>().c_str(), 0);
        gstlinkable::link(queue, colorspace);
        gstlinkable::link(colorspace, videosink);
    }
    if (enableAudio_)
    {
        LOG_DEBUG("Audio enabled");
        GstElement *queue = pipeline_->makeElement("queue", "audio_queue");
        GstElement *audioconvert = pipeline_->makeElement("audioconvert", 0);
        GstElement *audioresample = pipeline_->makeElement("audioresample", 0);
        GstElement *audiosink = pipeline_->makeElement(options["audiosink"].as<string>().c_str(), 0);
        g_object_set(audiosink, "buffer-time", options["audio-buffer"].as<int>() *
                USEC_PER_MILLISEC, NULL);
        gstlinkable::link(queue, audioconvert);
        gstlinkable::link(audioconvert, audioresample);
        gstlinkable::link(audioresample, audiosink);
    }
}

RTSPClient::~RTSPClient()
{}

void RTSPClient::run(int timeToLive)
{
    /* run */
    bool running = false;
    while (!running and not signal_handlers::signalFlag())
    {
        LOG_INFO("Waiting for rtsp server");
        if (not pipeline_->start())
        {
            LOG_WARNING("Failed to change state of pipeline");
            //pipeline_->makeNull();
            g_usleep(G_USEC_PER_SEC);
        }
        else
            running = true;
    }
    /* add a timeout to check the interrupted variable */
    g_timeout_add_seconds(5, (GSourceFunc) timeout, NULL);

    /* start main loop */
    if (not signal_handlers::signalFlag())
        gutil::runMainLoop(timeToLive);

    pipeline_->stop();

    LOG_DEBUG("Client exitting...\n");
}

