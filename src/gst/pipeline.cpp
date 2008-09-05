
// pipeline.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//
// NOTES:
// Change verbose_ to true if you want Gstreamer to tell you everything that's going on 
// in the pipeline

#include <gst/gst.h>

#include <cassert>
#include <unistd.h>
#include <cstdio>
#include "pipeline.h"
#include "logWriter.h"

Pipeline *Pipeline::instance_ = 0;

Pipeline::Pipeline()
    : pipeline_(0), startTime_(0), verbose_(false)
{
    // empty
}


Pipeline & Pipeline::Instance()
{
    if (instance_ == 0) {
        instance_ = new Pipeline();
        instance_->init();
    }
    return *instance_;
}


Pipeline::~Pipeline()
{
    assert(stop());
    gst_object_unref(GST_OBJECT(pipeline_));
}


gboolean Pipeline::bus_call(GstBus * /*bus*/, GstMessage *msg, gpointer /*data*/)
{
    //    GMainLoop *loop = data;
    //gboolean *done = static_cast<gboolean*>(data);

    switch(GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_UNKNOWN:
            break;
        case GST_MESSAGE_EOS:
            g_print("End-of-stream\n");
            //*done = TRUE;

            //g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: 
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_error(msg, &err, &debug);

                g_print("Error: %s\n", err->message);
                g_error_free(err);

                if (debug) {
                    g_print("Debug details: %s\n", debug);
                    g_free(debug);
                }

                //*done = TRUE;
                //g_main_loop_quit(loop);
                break;
            }
        case GST_MESSAGE_WARNING:
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_warning(msg, &err, &debug);

                //g_print("Warning: %s\n", err->message);
                LOG(err->message, WARNING);
                g_error_free(err);

                if (debug) {
                    g_print("Debug details: %s\n", debug);
                    g_free(debug);
                }
                break;
            }
        case GST_MESSAGE_INFO:
            break;
        case GST_MESSAGE_TAG:
            break;
        case GST_MESSAGE_BUFFERING:         
            break;
        case GST_MESSAGE_STATE_CHANGED:     
            break;
        case GST_MESSAGE_STATE_DIRTY: 
            break;
        case GST_MESSAGE_STEP_DONE:
            break;
        case GST_MESSAGE_CLOCK_PROVIDE:
            break;
        case GST_MESSAGE_CLOCK_LOST:
            break;
        case GST_MESSAGE_NEW_CLOCK:
            break;
        case GST_MESSAGE_STRUCTURE_CHANGE:
            break;
        case GST_MESSAGE_STREAM_STATUS:
            break;
        case GST_MESSAGE_APPLICATION:   
            break;
        case GST_MESSAGE_ELEMENT:
            break;
        case GST_MESSAGE_SEGMENT_START:
            break;
        case GST_MESSAGE_SEGMENT_DONE:
            break;
        case GST_MESSAGE_DURATION:
            break;
        case GST_MESSAGE_LATENCY:
            break;
        case GST_MESSAGE_ASYNC_START:       
            break;
        case GST_MESSAGE_ASYNC_DONE:      
            break;
        case GST_MESSAGE_ANY:
            break;
        default:
            break;
    }

    return TRUE;
}


void Pipeline::init()
{
    if (!pipeline_)
    {
        gst_init(0, NULL);
        assert(pipeline_ = gst_pipeline_new("pipeline"));

        if (verbose_)
            make_verbose();
        // this will be used as a reference for future
        // pipeline synchronization
        startTime_ = gst_clock_get_time(clock());

        /* watch for messages on the pipeline's bus (note that this will only
         *      * work like this when a GLib main loop is running) */
        GstBus *bus;
        bus = getBus();
        gst_bus_add_watch(bus, GstBusFunc(bus_call), static_cast<gpointer>(NULL));
        gst_object_unref(bus);
    }
}


// FIXME: check if this is safe, we're destroying and recreating the pipeline
void Pipeline::reset()
{
    if (pipeline_)
    {
        LOG("Pipeline is being reset.", DEBUG);
        assert(stop());
        delete instance_;
        instance_ = 0;
    }
}


void Pipeline::make_verbose()
{
    // Get verbose output
    if (verbose_) {
        gchar *exclude_args = NULL;     // set args to be excluded from output
        gchar **exclude_list = exclude_args ? g_strsplit(exclude_args, ",", 0) : NULL;
        g_signal_connect(pipeline_, "deep_notify",
                G_CALLBACK(gst_object_default_deep_notify), exclude_list);
    }
}


bool Pipeline::isPlaying() const
{
    if (pipeline_ && (GST_STATE(pipeline_) == GST_STATE_PLAYING))
        return true;
    else
        return false;
}


void Pipeline::wait_until_playing() const
{
    while (!isPlaying())
        usleep(10000);
}


void Pipeline::wait_until_stopped() const
{
    while (isPlaying())
        usleep(10000);
}


bool Pipeline::checkStateChange(GstStateChangeReturn ret)
{
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("Failed to start pipeline!\n");
        GstBus *bus;
        bus = getBus();

        /* check if there is an error message with details on the bus */
        GstMessage *msg = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
        if (msg) {
            GError *err = NULL;

            gst_message_parse_error(msg, &err, NULL);
            g_print("ERROR: %s\n", err->message);
            g_error_free(err);
            gst_message_unref(msg);
        }

        gst_object_unref(bus);
        return false;
    }
    else
        return true;
}

bool Pipeline::start()
{
    checkStateChange(gst_element_set_state(pipeline_, GST_STATE_PAUSED)); // set it to paused 
    assert(checkStateChange(gst_element_set_state(pipeline_, GST_STATE_PLAYING))); // set it to playing
    return isPlaying();
}


bool Pipeline::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    return !isPlaying();
}


void Pipeline::add(GstElement *element)
{
    gst_bin_add(GST_BIN(pipeline_), element);
}


void Pipeline::add(std::vector<GstElement*> &elementVec)
{
    std::vector< GstElement * >::iterator iter;
    for (iter = elementVec.begin(); iter != elementVec.end(); ++iter)
        add(*iter);
}


void Pipeline::remove(GstElement *element)
{
    if (element)
        assert(gst_bin_remove(GST_BIN(pipeline_), element));
}


void Pipeline::remove(std::vector<GstElement*> &elementVec)
{
    std::vector<GstElement *>::iterator iter;
    for (iter = elementVec.begin(); iter != elementVec.end(); ++iter)
        remove(*iter);
}


GstClockID Pipeline::add_clock_callback(GstClockCallback callback, gpointer user_data)
{
    GstClockID clockId = gst_clock_new_periodic_id(clock(), startTime_, GST_SECOND);
    gst_clock_id_wait_async(clockId, callback, user_data);
    return clockId;
}


void Pipeline::remove_clock_callback(GstClockID clockId)
{
    gst_clock_id_unschedule(clockId);
    gst_clock_id_unref(clockId);
}


