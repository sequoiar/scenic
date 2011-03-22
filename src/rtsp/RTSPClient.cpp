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
#include "gst/messageDispatcher.h"
#include "gst/videoConfig.h"
#include "gst/videoScale.h"
#include "gst/videoFlip.h"
#include "gst/videoSink.h"
#include "gst/textOverlay.h"
#include "gst/gstLinkable.h"

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

bool RTSPClient::validPortRange(const std::string &ports)
{
    std::vector<std::string> strs;
    boost::split(strs, ports, boost::is_any_of("- "));
    if (strs.size() != 2)
        return false;
    int first = boost::lexical_cast<int>(strs[0]);
    int second = boost::lexical_cast<int>(strs[1]);
    static const int MINIMUM_PORT_RANGE = enableVideo_ and enableAudio_ ? 5 : 3; // RTP=n, RTCP1=n+1, RTCP2=n+3
    if (first >= 1 and first <= 65535 and second >= 1 and second <= 65535)
        if ((second - first) >= MINIMUM_PORT_RANGE)
            return true;
    return false;
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

void RTSPClient::linkNewPad(GstPad *newPad, const GstCaps *caps, const gchar *queue_name)
{
    GstElement *queue = pipeline_->findElementByName(queue_name);
    if (queue == 0) 
    {
        LOG_WARNING("No element named " << queue_name << ", not linking");
        return;
    }
    GstPad *sinkPad = gst_element_get_static_pad(queue, "sink");
    if (GST_PAD_IS_LINKED(sinkPad))
    {
        GstObject *parent = GST_OBJECT (GST_OBJECT_PARENT (sinkPad));
        LOG_WARNING("Omitting link for pad " << GST_OBJECT_NAME(parent) << 
                ":" << GST_OBJECT_NAME(sinkPad) << " because it's already linked");
        gst_object_unref (GST_OBJECT (sinkPad));
        return;
    }
    /* can it link to the new pad? */
    GstCaps *sinkCaps = gst_pad_get_caps (sinkPad);
    GstCaps *res = gst_caps_intersect (caps, sinkCaps);
    bool linked = false;
    if (res && !gst_caps_is_empty (res)) 
    {
        LOG_DEBUG("Found pad to link to pipeline - plugging is now done");
        linked = gstlinkable::link_pads(newPad, sinkPad);
    }
    if (not linked) 
        LOG_WARNING("Could not link new pad to pipeline");
    gst_caps_unref (sinkCaps);
    gst_caps_unref (res);
    gst_object_unref (GST_OBJECT (sinkPad));
}

void RTSPClient::onPadAdded(GstElement * /*uridecodebin*/, GstPad *newPad, gpointer data)
{
    RTSPClient *context = static_cast<RTSPClient*>(data);
    GstCaps *caps = gst_pad_get_caps (newPad);
    const gchar *mime = gst_structure_get_name (gst_caps_get_structure (caps, 0));
    if (g_strrstr (mime, "video"))
    {
        if (context->enableVideo_)
            context->linkNewPad(newPad, caps, "video_queue");
        else
            LOG_WARNING("Got video stream even though we've disabled video");
    }
    else if (g_strrstr (mime, "audio"))
    {
        if (context->enableAudio_)
            context->linkNewPad(newPad, caps, "audio_queue");
        else
            LOG_WARNING("Got audio stream even though we've disabled audio");
    }
    else
        LOG_WARNING("Got unknown mimetype " << mime);
    gst_caps_unref (caps);
}

static const int USEC_PER_MILLISEC = G_USEC_PER_SEC / 1000.0;

RTSPClient::RTSPClient(const boost::program_options::variables_map &options, 
        bool enableVideo, 
        bool enableAudio) :
    BusMsgHandler(),
    pipeline_(new Pipeline), 
    latencySet_(false), 
    portRange_(""),
    latency_(options["jitterbuffer"].as<int>()), 
    enableVideo_(enableVideo), 
    enableAudio_(enableAudio),
    fullscreenAtStartup_(options["fullscreen"].as<bool>()),
    windowTitle_(options["window-title"].as<std::string>())
{
    using std::string;
    BusMsgHandler::setPipeline(pipeline_.get()); // this is how we subscribe to the pipeline's bus messages

    VideoSinkConfig vConfig(options);
    videoscale_.reset(vConfig.createVideoScale(*pipeline_));
    textoverlay_.reset(vConfig.createTextOverlay(*pipeline_));
    videoflip_.reset(vConfig.createVideoFlip(*pipeline_));
    videosink_.reset(vConfig.createSink(*pipeline_));

    if (options["debug"].as<string>() == "gst-debug")
        pipeline_->makeVerbose();

    GstElement *uridecodebin = pipeline_->makeElement("uridecodebin", "decode");
    string uri("rtsp://" + options["address"].as<string>() + ":8554/test");
    g_object_set(uridecodebin, "uri", uri.c_str(), NULL);
    g_signal_connect(uridecodebin, "notify::source", G_CALLBACK(onNotifySource), this);
    g_signal_connect(uridecodebin, "pad-added", G_CALLBACK(onPadAdded), this);

    // get port range
    if (options.count("port-range"))
    {
        if (validPortRange(options["port-range"].as<string>()))
            portRange_ = options["port-range"].as<string>();
        else
            LOG_WARNING("Invalid port-range " << options["port-range"].as<string>() << ", ignoring.");
    }

    if (enableVideo_)
    {
        LOG_DEBUG("Video enabled");
        GstElement *queue = pipeline_->makeElement("queue", "video_queue");
        GstElement *colorspace = pipeline_->makeElement("ffmpegcolorspace", 0);
        gstlinkable::link(queue, colorspace);
        gstlinkable::link(colorspace, *videoscale_);
        gstlinkable::link(*videoscale_, *textoverlay_);
        gstlinkable::link(*textoverlay_, *videoflip_);
        gstlinkable::link(*videoflip_, *videosink_);
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

void RTSPClient::run(int timeToLive)
{
    /* run */
    while (!pipeline_->isPlaying() and not signal_handlers::signalFlag())
    {
        LOG_INFO("Waiting for rtsp server");
        pipeline_->start();
        g_usleep(G_USEC_PER_SEC); // sleep a bit
    }
    /* add a timeout to check the interrupted variable */
    g_timeout_add_seconds(5, (GSourceFunc) timeout, NULL);
    
    if (enableVideo_)
    {
        if(fullscreenAtStartup_)
            MessageDispatcher::sendMessage("fullscreen");
        MessageDispatcher::sendMessage("window-title", windowTitle_);
    }

    /* start main loop */
    if (not signal_handlers::signalFlag())
        gutil::runMainLoop(timeToLive);

    pipeline_->stop();

    LOG_DEBUG("Client exitting...\n");
}

bool RTSPClient::handleBusMsg(GstMessage *msg)
{
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
    {
        LOG_DEBUG("Got end of stream, quitting the main loop");
        gutil::killMainLoop();
    }
    return false; // other objects might want this message
}
