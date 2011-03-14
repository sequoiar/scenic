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

#include "logWriter.h"
#include "gtk_utils.h"

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
                // FIXME: this never works!
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

gboolean
RTSPClient::timeout (RTSPClient *client)
{
    // this will be false until the recalculate latency call is made in the bus callback
    if (!client->latencySet_)
        g_object_set(client->rtpbin_, "latency", 5, NULL);
    else
        return FALSE; // don't call again if we've already recalculated latency

    return TRUE;
}

RTSPClient::RTSPClient(const boost::program_options::variables_map &options, bool enableVideo, bool enableAudio) :
    rtpbin_(0), pipeline_(0), latencySet_(false)
{
    using std::string;
    string launchLine("uridecodebin uri=rtsp://localhost:8554/test name=decode ");
    if (enableVideo)
        launchLine += " decode. ! queue ! ffmpegcolorspace ! timeoverlay halignment=right ! " + options["videosink"].as<string>(); 
    if (enableAudio)
        launchLine += " decode. ! queue ! audioconvert ! autoaudiosink";

    pipeline_ = gst_parse_launch(launchLine.c_str(), NULL /* TODO add error checking */);

    // add bus call
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
    gst_bus_add_watch(bus, busCall, this);
    gst_object_unref(bus);
}
        
void RTSPClient::run(int timeToLive)
{
    /* run */
    GstStateChangeReturn ret = gst_element_set_state (pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
        THROW_CRITICAL("Failed to change state of pipeline");
    LOG_INFO("Waiting for rtsp server");
    while (rtpbin_ == 0) 
    {
        rtpbin_ = gst_bin_get_by_name (GST_BIN(pipeline_),
                "rtpbin0");
        usleep(1000);
    }
    /* add a timeout to check the interrupted variable */
    g_timeout_add_seconds(5, (GSourceFunc) timeout, this);

    /* start main loop */
    gutil::runMainLoop(timeToLive);

    /* clean up */
    gst_element_set_state (pipeline_, GST_STATE_NULL);
    gst_object_unref (pipeline_);

    LOG_DEBUG("Client exitting...\n");
}

