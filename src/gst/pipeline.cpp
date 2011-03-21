/* pipeline.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
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

#include "pipeline.h"
#include "util/logWriter.h"
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "gtk_utils.h"

#include "dv1394.h"
#include "busMsgHandler.h"


// NOTES:
// Change verbose_ to true if you want Gstreamer to tell you everything that's going on
// in the pipeline

Pipeline::Pipeline() : pipeline_(gst_pipeline_new("pipeline")), handlers_(), 
    sampleRate_(SAMPLE_RATE)
{
    /* watch for messages on the pipeline's bus (note that this will only
     *      work like this when a GLib main loop is running) */
    GstBus *bus = getBus();
    gst_bus_add_watch(bus, static_cast<GstBusFunc>(bus_call), static_cast<gpointer>(this));
    gst_object_unref(bus);
}

Pipeline::~Pipeline()
{
    Dv1394::reset();
    LOG_DEBUG("Unreffing pipeline");
    gst_object_unref(GST_OBJECT(pipeline_));
}


namespace {
/// Translate error messages into more helpful/detailed info
void translateMessage(GstObject *src, const std::string &errStr)
{
    // FIXME: somehow this info could be improved by getting details from elsewhere,
    // or at least querying the element responsible for more info
    std::string srcName = gst_object_get_name(src);
    if (srcName.find("udpsrc") != std::string::npos) // this comes from a udpsrc
    {
        if (errStr.find("Could not get/set settings from/on resource") != std::string::npos)
        {
            int port;
            g_object_get(src, "port", &port, NULL);

            THROW_CRITICAL(srcName << ":" << errStr << " Port " <<
                boost::lexical_cast<std::string>(port) << " may be in use by another process.");
        }
    }
    else if (srcName.find("v4l2src") != std::string::npos) // this comes from a v4l2src
    {
        static const std::string v4l2busy("Could not enqueue buffers in device ");
        size_t pos = errStr.find(v4l2busy);
        if (pos != std::string::npos)
        {
            std::string deviceName(errStr.substr(pos + v4l2busy.length(), errStr.length() - v4l2busy.length() - 1));
            THROW_CRITICAL(srcName << ":" << errStr <<
                 deviceName << " is probably already in use.");
        }
        else 
            THROW_CRITICAL(srcName << ":" << errStr);
        return;
    }

    LOG_WARNING(srcName << ":" << errStr);
}
}


gboolean Pipeline::bus_call(GstBus * /*bus*/, GstMessage *msg, gpointer data)
{
    Pipeline *context = static_cast<Pipeline*>(data);
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            {
                LOG_DEBUG("End-of-stream");
                context->updateListeners(msg); // forward message to subscribers
                break;
            }
        case GST_MESSAGE_ERROR:
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_error(msg, &err, &debug);

                std::string errStr(err->message);
                g_error_free(err);

                if (debug) {
                    LOG_DEBUG("Debug details: " << debug);
                    g_free(debug);
                }
                // this will either throw or log a warning
                translateMessage(msg->src, errStr); 
                break;
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
        case GST_MESSAGE_ELEMENT:
            context->updateListeners(msg);
            break;
        case GST_MESSAGE_APPLICATION:
            context->updateListeners(msg);
            break;
        case GST_MESSAGE_LATENCY:
            {
                LOG_DEBUG("Latency change, recalculating latency for pipeline");
                // when pipeline latency is changed, this msg is posted on the bus. we then have
                // to explicitly tell the pipeline to recalculate its latency
                if (!gst_bin_recalculate_latency (GST_BIN_CAST (context->pipeline_)))
                    LOG_WARNING("Could not reconfigure latency.");
                break;
            }

        default:
            break;
    }

    return TRUE;
}


void deepNotifyCb(GObject * /*object*/, GstObject * orig, GParamSpec * pspec, gchar **excluded_props)
{
    GValue value; /* the important thing is that value.type = 0 */
    memset(&value, 0, sizeof(value));
    gchar *str = NULL;
    gchar *name = NULL;

    if (pspec->flags & G_PARAM_READABLE) 
    {
        /* let's not print these out for excluded properties... */
        while (excluded_props != NULL && *excluded_props != NULL) 
        {
            if (g_strcmp0 (pspec->name, *excluded_props) == 0)
                return;
            excluded_props++;
        }
        g_value_init (&value, G_PARAM_SPEC_VALUE_TYPE (pspec));
        g_object_get_property (G_OBJECT (orig), pspec->name, &value);

        /* FIXME: handle flags */
        if (G_IS_PARAM_SPEC_ENUM (pspec)) 
        {
            GEnumValue *enum_value;
            GEnumClass *klass = G_ENUM_CLASS (g_type_class_ref (pspec->value_type));
            enum_value = g_enum_get_value (klass, g_value_get_enum (&value));

            str = g_strdup_printf ("%s (%d)", enum_value->value_nick,
                    enum_value->value);
            g_type_class_unref (klass);
        } 
        else 
            str = g_strdup_value_contents (&value);

        name = gst_object_get_path_string (orig);
        LOG_DEBUG(name << ": " << pspec->name << " = " << str);
        g_free(name);
        g_free(str);
        g_value_unset (&value);
    } 
    else 
    {
        name = gst_object_get_path_string (orig);
        LOG_WARNING("Parameter " << pspec->name << " not readable in " << name << ".");
        g_free (name);
    }
}


void Pipeline::makeVerbose() const
{
    // Get verbose output
    gchar *exclude_args = NULL;     // set args to be excluded from output
    gchar **exclude_list = exclude_args ? g_strsplit(exclude_args, ",", 0) : NULL;
    g_signal_connect(pipeline_, "deep_notify",
            G_CALLBACK(deepNotifyCb), exclude_list);
}


bool Pipeline::isPlaying() const
{
    if (GST_STATE(pipeline_) == GST_STATE_PLAYING)
        return true;
    else
        return false;
}


bool Pipeline::isReady() const
{
    if (pipeline_ and (GST_STATE(pipeline_) == GST_STATE_READY))
        return true;
    else
        return false;
}


bool Pipeline::isPaused() const
{
    if (GST_STATE(pipeline_) == GST_STATE_PAUSED)
        return true;
    else
        return false;
}


bool Pipeline::isStopped() const
{
    if (GST_STATE(pipeline_) == GST_STATE_NULL)
        return true;
    else
        return false;
}

namespace {
bool checkStateChange(GstStateChangeReturn ret)
{
    switch (ret)
    {
        case GST_STATE_CHANGE_SUCCESS:
        {
            LOG_DEBUG("Element state change was successful");
            return true;
        }
        case GST_STATE_CHANGE_ASYNC:
        {
            LOG_DEBUG("Element will change state asynchronously");
            return true;
        }
        case GST_STATE_CHANGE_NO_PREROLL:
        {
            LOG_DEBUG("Element is live, no preroll");
            return true;
        }
        case GST_STATE_CHANGE_FAILURE:
        {
            LOG_WARNING("Failed to change state of pipeline");
            return false;
        }
        default:
            return false;
    }
}
}


bool Pipeline::start() const
{
    if (isPlaying())        // only needs to be started once
    {
        LOG_WARNING("Already playing, this call to Pipeline::start() was redundant"); 
        return true;
    }
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (not checkStateChange(ret)) // set it to playing
    {
        LOG_WARNING("Could not set pipeline state to PLAYING");
        return false;
    }

    LOG_DEBUG("Now playing");
    GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(pipeline_), GST_DEBUG_GRAPH_SHOW_ALL, "milhouse");
    return true;
}



void Pipeline::makeReady() const
{
    if (isReady())        // only needs to be started once
        return;
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_READY);
    if (checkStateChange(ret)) 
        LOG_WARNING("Could not set pipeline state to READY");
    else
        LOG_DEBUG("Now ready");
}

bool Pipeline::makeNull() const
{
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_NULL);
    if (not checkStateChange(ret)) 
    {
        LOG_WARNING("Could not set pipeline state to NULL");
        return false;
    }
    else
    {
        LOG_DEBUG("pipeline state Now NULL");
        return true;
    }
}


void Pipeline::pause() const
{
    if (isPaused())        // only needs to be paused once
        return;
    makeReady();
    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PAUSED);
    if (not checkStateChange(ret)) // set it to paused
        THROW_ERROR("Could not set pipeline state to PAUSED");
    else
        LOG_DEBUG("Now paused");
}


void Pipeline::quit() const
{
    stop();
    gutil::killMainLoop();
}


void Pipeline::stop() const
{
    if (isStopped())        // only needs to be stopped once
        return;
    else if (pipeline_)
    {
        if (not makeNull())
            LOG_WARNING("Could not set pipeline state to NULL");
        else
            LOG_DEBUG("Now stopped/null");
    }
    else
        LOG_WARNING("PIPELINE == 0!");
}


void Pipeline::add(GstElement *element) const
{
    gst_bin_add(GST_BIN(pipeline_), element);
}

GstBus* Pipeline::getBus() const
{
    return gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
}

GstElement *Pipeline::findElementByName(const char *elementName)
{
    GstElement *element = gst_bin_get_by_name(GST_BIN(pipeline_), elementName);
    if (element == 0)
        LOG_WARNING("No element named " << elementName);

    return element;
}

GstElement *Pipeline::makeElement(const char *factoryName, const char *elementName) const
{
    GstElement *element = gst_element_factory_make(factoryName, elementName);
    if (element == 0)
        THROW_ERROR("No such element or plugin " << factoryName <<
                ".Check that all necessary plugins are installed with " <<
                "gst-check.py");

    add(element);
    return element;
}


void Pipeline::subscribe(BusMsgHandler *obj)
{
    handlers_.insert(obj);
}


/// Remove the busmsghandler from the list
void Pipeline::unsubscribe(BusMsgHandler *obj)
{
    handlers_.erase(obj);
}


void Pipeline::updateListeners(GstMessage *msg)
{
    // TODO: are we guaranteed that these are in a callable state?
    // this loop goes through our listeners until it finds one that returns
    // true upon receiving this message
    for (std::set<BusMsgHandler*>::iterator iter = handlers_.begin(); 
            iter != handlers_.end(); ++iter)
        if ((*iter)->handleBusMsg(msg))
            break;
}


void Pipeline::seekTo(gint64 pos)
{
    if (!gst_element_seek(pipeline_, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                GST_SEEK_TYPE_SET, pos,
                GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
        THROW_ERROR("Seek failed!");
    }
}


void Pipeline::updateSampleRate(unsigned newRate)
{
    LOG_INFO("Updating sample rate to " << newRate);
    sampleRate_ = newRate;
}


unsigned Pipeline::actualSampleRate() const
{
    return sampleRate_;
}

