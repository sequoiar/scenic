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
            break;
    }

    return TRUE;
}

    namespace {
    void print_pipeline(GstElement *pipeline)
    {
        gboolean done = FALSE;
        gpointer item;
        GstIterator *it = gst_bin_iterate_recurse(GST_BIN(pipeline));
        while (!done) {
            switch (gst_iterator_next (it, &item)) {
                case GST_ITERATOR_OK:
                    if (g_strcmp0("decode", gst_object_get_name(GST_OBJECT(item))) == 0) {
                        g_print("decode has %d childproxy elements\n", gst_child_proxy_get_children_count(GST_CHILD_PROXY(item)));
                        gboolean ddone = FALSE;
                        gpointer ditem;
                        GstIterator *dit = gst_bin_iterate_elements(GST_BIN(item));

                        while (!ddone) {
                            switch (gst_iterator_next (dit, &ditem)) {
                                case GST_ITERATOR_OK:
                                    g_print("%s", G_OBJECT_CLASS_NAME(G_OBJECT(ditem)));
                                    gst_object_unref (ditem);
                                    break;
                                case GST_ITERATOR_RESYNC:
                                    gst_iterator_resync (dit);
                                    break;
                                case GST_ITERATOR_ERROR:
                                    ddone = TRUE;
                                    break;
                                case GST_ITERATOR_DONE:
                                    ddone = TRUE;
                                    break;
                            }
                        }
                    }
                    else
                        g_print("%s\n", gst_object_get_name(GST_OBJECT(item)));
                    gst_object_unref (item);
                    break;
                case GST_ITERATOR_RESYNC:
                    gst_iterator_resync (it);
                    break;
                case GST_ITERATOR_ERROR:
                    done = TRUE;
                    break;
                case GST_ITERATOR_DONE:
                    done = TRUE;
                    break;
            }
        }
        gst_iterator_free (it);
    }
}


gboolean
RTSPClient::timeout (RTSPClient * client)
{
    // this will be false until the recalculate latency call is made in the bus callback
    if (!client->latencySet_)
        g_object_set(client->rtpbin_, "latency", 15, NULL);
    else
        return FALSE; // don't call again if we've already recalculated latency

    print_pipeline(client->pipeline_);
    return TRUE;
}

RTSPClient::RTSPClient(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio) :
    rtpbin_(0), pipeline_(0), latencySet_(false)
{
    using std::string;
    string launchLine("uridecodebin uri=rtsp://");
    launchLine += options["address"].as<string>(); // i.e. localhost
    launchLine += ":8554/test name=decode ";

    if (enableVideo)
    {
        LOG_DEBUG("Video enabled");
        launchLine += " decode. ! queue ! ffmpegcolorspace ! timeoverlay halignment=right ! " + options["videosink"].as<string>(); 
    }
    if (enableAudio)
    {
        LOG_DEBUG("Audio enabled");
        launchLine += " decode. ! queue ! audioconvert ! audioresample ! " + options["audiosink"].as<string>();
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
}

void RTSPClient::run(int timeToLive)
{
    /* run */
    bool running = false;
    while (!running and !signal_handlers::signalFlag())
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

    while (rtpbin_ == 0 and !signal_handlers::signalFlag()) 
    {
        rtpbin_ = gst_bin_get_by_name (GST_BIN(pipeline_),
                "rtpbin0");
        g_usleep(G_USEC_PER_SEC);
    }
    LOG_DEBUG("Got rtpbin");
    /* add a timeout to check the interrupted variable */
    g_timeout_add_seconds(5, (GSourceFunc) timeout, this);

    GstObject *obj = 0;
    GParamSpec *pspec;
    bool result = gst_child_proxy_lookup (GST_OBJECT(pipeline_), "port-range", &obj, &pspec);
    if (obj)
        gst_object_unref(obj);
    if (result)
        LOG_INFO("Found property port-range");

    /* start main loop */
    gutil::runMainLoop(timeToLive);

    /* clean up */
    gst_element_set_state (pipeline_, GST_STATE_NULL);
    gst_object_unref (pipeline_);

    LOG_DEBUG("Client exitting...\n");
}

