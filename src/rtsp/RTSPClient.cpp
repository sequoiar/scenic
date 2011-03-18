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

static const int USEC_PER_MILLISEC = G_USEC_PER_SEC / 1000.0;

RTSPClient::RTSPClient(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio) :
    pipeline_(0), latencySet_(false), portRange_(""), latency_(options["jitterbuffer"].as<int>())
{
    using std::string;
    string launchLine("uridecodebin uri=rtsp://");
    launchLine += options["address"].as<string>(); // i.e. localhost
    launchLine += ":8554/test name=decode ";

    // get port range
    if (options.count("port-range"))
    {
        if (validPortRange(options["port-range"].as<std::string>()))
            portRange_ = options["port-range"].as<std::string>();
        else
            LOG_WARNING("Invalid port-range " << options["port-range"].as<std::string>() << ", ignoring.");
    }

    if (enableVideo)
    {
        LOG_DEBUG("Video enabled");
        launchLine += " decode. ! queue ! ffmpegcolorspace ! timeoverlay halignment=right ! " + options["videosink"].as<string>(); 
    }
    if (enableAudio)
    {
        LOG_DEBUG("Audio enabled");
        launchLine += " decode. ! queue ! audioconvert ! audioresample ! " + 
            options["audiosink"].as<string>() + " buffer-time=" + 
            boost::lexical_cast<string>(options["audio-buffer"].as<int>() * USEC_PER_MILLISEC);
    }

    GError *error = NULL;
    pipeline_ = gst_parse_launch(launchLine.c_str(), &error);
    if (error != NULL) 
    {
        /* a recoverable error was encountered */
        LOG_WARNING("recoverable parsing error: " << error->message);
        g_error_free (error);
    }
    if (pipeline_ == 0)
        THROW_CRITICAL("Could not create pipeline from description " << launchLine);

    // add bus call
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_add_watch(bus, busCall, this);
    gst_object_unref(bus);

    GstElement *decodebin = gst_bin_get_by_name (GST_BIN(pipeline_),
                "decode");
    g_signal_connect(decodebin, "notify::source", G_CALLBACK(onNotifySource), this);
}

RTSPClient::~RTSPClient()
{
    /* clean up */
    if (pipeline_)
    {
        gst_element_set_state (pipeline_, GST_STATE_NULL);
        gst_object_unref (pipeline_);
    }
}

void RTSPClient::run(int timeToLive)
{
    /* run */
    bool running = false;
    while (!running and not signal_handlers::signalFlag())
    {
        LOG_INFO("Waiting for rtsp server");
        GstStateChangeReturn ret = gst_element_set_state (pipeline_, GST_STATE_PLAYING);
        if (ret == GST_STATE_CHANGE_FAILURE)
        {
            LOG_WARNING("Failed to change state of pipeline");
            gst_element_set_state (pipeline_, GST_STATE_NULL);
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

    LOG_DEBUG("Client exitting...\n");
}

