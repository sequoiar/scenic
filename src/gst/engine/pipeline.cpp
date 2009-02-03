/* pipeline.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

#include "util.h"

#include <gst/gst.h>
#include "pipeline.h"
#include "dv1394.h"
#include "busMsgHandler.h"

// NOTES:
// Change verbose_ to true if you want Gstreamer to tell you everything that's going on
// in the pipeline

Pipeline *Pipeline::instance_ = 0;

const unsigned int Pipeline::SAMPLE_RATE = 48000;

Pipeline * Pipeline::Instance()
{
    if (instance_ == 0) {
        instance_ = new Pipeline();
        instance_->init();
    }
    return instance_;
}


Pipeline::~Pipeline()
{
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}


gboolean Pipeline::bus_call(GstBus * /*bus*/, GstMessage *msg, gpointer /*data*/)
{
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            {
                LOG_DEBUG("End-of-stream");
                Instance()->updateListeners(msg);
                break;
            }
        case GST_MESSAGE_ERROR:
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_error(msg, &err, &debug);

                LOG_WARNING(err->message);
                g_error_free(err);

                if (debug) {
                    LOG_DEBUG("Debug details: " << debug);
                    g_free(debug);
                }
                break;
            }
        case GST_MESSAGE_WARNING:
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_warning(msg, &err, &debug);

                LOG_WARNING(err->message);
                g_error_free(err);

                if (debug) {
                    LOG_DEBUG("Debug details: " << debug);
                    g_free(debug);
                }
                break;
            }
        case GST_MESSAGE_ELEMENT:
            {
                Instance()->updateListeners(msg);
                break;
            }
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

        // this will be used as a reference for future
        // pipeline synchronization
        startTime_ = gst_clock_get_time(clock());

        /* watch for messages on the pipeline's bus (note that this will only
         *      work like this when a GLib main loop is running) */
        GstBus *bus;
        bus = getBus();
        gst_bus_add_watch(bus, GstBusFunc(bus_call), static_cast<gpointer>(this));
        gst_object_unref(bus);
    }
}


// TODO: check if this is safe, we're destroying and recreating the pipeline
// This can be a class method or a member method, it's a class method for the sake of 
// looking like Instance()

void Pipeline::reset()
{
    if (instance_)
    {
        LOG_DEBUG("Pipeline is being reset.");
        instance_->stop();
        delete instance_;
        instance_ = 0;
    }
}


void Pipeline::makeVerbose()
{
    // Get verbose output
    gchar *exclude_args = NULL;     // set args to be excluded from output
    gchar **exclude_list = exclude_args ? g_strsplit(exclude_args, ",", 0) : NULL;
    g_signal_connect(pipeline_, "deep_notify",
            G_CALLBACK(gst_object_default_deep_notify), exclude_list);
}


bool Pipeline::isPlaying() const
{
    if (pipeline_ && (GST_STATE(pipeline_) == GST_STATE_PLAYING))
        return true;
    else
        return false;
}


bool Pipeline::isPaused() const
{
    if (pipeline_ && (GST_STATE(pipeline_) == GST_STATE_PAUSED))
        return true;
    else
        return false;
}

bool Pipeline::checkStateChange(GstStateChangeReturn ret) const
{
    if (ret == GST_STATE_CHANGE_NO_PREROLL)
    {
        LOG_DEBUG("Element is live, no preroll");
        return true;
    }
    else if (ret == GST_STATE_CHANGE_FAILURE) {
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


void Pipeline::start()
{
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    assert(checkStateChange(ret)); // set it to playing
    LOG_DEBUG("Now playing");
}


void Pipeline::pause()
{
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PAUSED);
    assert(checkStateChange(ret)); // set it to paused
    LOG_DEBUG("Now paused");
}


void Pipeline::stop()
{
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_NULL);
    assert(checkStateChange(ret)); // set it to paused
    LOG_DEBUG("Now stopped/null");
}


void Pipeline::add(GstElement *element)
{
    gst_bin_add(GST_BIN(pipeline_), element);
    ++refCount_;
}


void Pipeline::remove(GstElement **element) // guarantees that original pointer will be zeroed
{                                           // and not reusable
    stop();
    if (*element)
    {
        assert(gst_bin_remove(GST_BIN(pipeline_), *element));
        *element = NULL;
        --refCount_;

        if (refCount_ <= 0)
        {
            assert(refCount_ == 0);
            Dv1394::reset();
            reset();
        }
    }
}


void Pipeline::remove(std::vector<GstElement*> &elementVec)
{
    stop();
    std::vector<GstElement *>::iterator iter;
    if (!elementVec.empty())
    {
        for (iter = elementVec.begin(); iter != elementVec.end(); ++iter)
        {
            if (*iter)
            {
                assert(gst_bin_remove(GST_BIN(pipeline_), *iter));
                *iter = NULL;
                --refCount_;
            }
        }
        if (refCount_ <= 0)
        {
            assert(refCount_ == 0);
            reset();
        }
    }
}


GstClockID Pipeline::add_clock_callback(GstClockCallback callback, gpointer user_data)
{
    GstClockID clockId = gst_clock_new_periodic_id(clock(), startTime_, GST_SECOND);
    gst_clock_id_wait_async(clockId, callback, user_data);
    return clockId;
}


void Pipeline::remove_clock_callback(GstClockID clockId)
{
    stop();
    gst_clock_id_unschedule(clockId);
    gst_clock_id_unref(clockId);
}


GstBus* Pipeline::getBus() const
{
    return gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
}


GstClock* Pipeline::clock() const
{
    return gst_pipeline_get_clock(GST_PIPELINE(pipeline_));
}

GstElement *Pipeline::makeElement(const char *factoryName, const char *elementName) 
{
    GstElement *element = gst_element_factory_make(factoryName, elementName);
    assert(element);
    add(element);
    return element;
}

GstElement *Pipeline::findElement(const char *name) const
{
    return gst_bin_get_by_name(GST_BIN(pipeline_), name);
}


void Pipeline::subscribe(BusMsgHandler *obj)
{
    handlers_.push_back(obj);
}


void Pipeline::updateListeners(GstMessage *msg)
{
    for (std::vector<BusMsgHandler*>::iterator iter = handlers_.begin(); 
            iter != handlers_.end(); ++iter)
        if ((*iter)->handleBusMsg(msg))
            break;
}


void Pipeline::seekTo(gint64 pos)
{
    if (!gst_element_seek (pipeline_, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                GST_SEEK_TYPE_SET, pos,
                GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
        THROW_ERROR("Seek failed!\n");
    }
}


const char* Pipeline::getElementPadCaps(GstElement *element, const char * padName) const
{
    assert(isPlaying() || isPaused());

    GstPad *pad;
    GstCaps *caps;

    assert(pad = gst_element_get_pad(GST_ELEMENT(element), padName));

    do
        caps = gst_pad_get_negotiated_caps(pad);
    while (caps == NULL);

    // goes until caps are initialized

    gst_object_unref(pad);

    const char *result = gst_caps_to_string(caps);
    gst_caps_unref(caps);
    return result;
}

