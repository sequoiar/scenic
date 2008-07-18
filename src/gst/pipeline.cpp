
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

#include <gst/gst.h>

#include <cassert>

#include "pipeline.h"
#include "logWriter.h"

Pipeline *Pipeline::instance_ = 0;

Pipeline::Pipeline() : pipeline_(0), verbose_(false)
{
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
    gst_object_unref(GST_OBJECT(pipeline_));
}

void Pipeline::init()
{
    if (!pipeline_) 
    {
        gst_init(0, NULL);
        pipeline_ = gst_pipeline_new("pipeline");
        assert(pipeline_);

        if (verbose_)
            make_verbose();

        startTime_ = gst_clock_get_time(clock());
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

void Pipeline::cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, void *data)
{
    std::cout << "Element: " << gst_element_get_name(srcElement) << std::endl;
    std::cout << "Pad: " << gst_pad_get_name(srcPad) << std::endl;
    if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
    {   
        LOG("Dynamic pad creation: pad is not a source");
        return;
    }

    GstElement *sinkElement = (GstElement *) data;
    GstPad *sinkPad;
    LOG("Dynamic pad created, linking new srcpad to existing sinkpad.");

    sinkPad = gst_element_get_static_pad(sinkElement, "sink");
    assert(gst_pad_link(srcPad, sinkPad) == GST_PAD_LINK_OK);
    gst_object_unref(sinkPad);
}

void Pipeline::cb_new_sink_pad(GstElement * sinkElement, GstPad * sinkPad, void *data)
{
    std::cout << "Element: " << gst_element_get_name(sinkElement) << std::endl;
    std::cout << "Pad: " << gst_pad_get_name(sinkPad) << std::endl;
    if (gst_pad_get_direction(sinkPad) != GST_PAD_SINK)
    {
        LOG("Dynamic pad creation: pad is not a sink");
        return;
    }

    GstElement *srcElement = (GstElement *) data;
    GstPad *srcPad;
    LOG("Dynamic pad created, linking new sinkpad to existing srcpad.");

    srcPad = gst_element_get_static_pad(srcElement, "src");
    assert(gst_pad_link(sinkPad, srcPad) == GST_PAD_LINK_OK);
    gst_object_unref(srcPad);
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
        usleep(1000);
}

bool Pipeline::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    return isPlaying();
}

bool Pipeline::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    return !isPlaying();
}

void Pipeline::add(GstElement * element)
{
    gst_bin_add(GST_BIN(pipeline_), element);
}

void Pipeline::add_vector(std::vector < GstElement * >&elementVec)
{
    std::vector < GstElement * >::iterator iter;
    for (iter = elementVec.begin(); iter != elementVec.end(); iter++)
        gst_bin_add(GST_BIN(pipeline_), *iter);
}

void Pipeline::remove(GstElement * element)
{
    if (element)
        assert(gst_bin_remove(GST_BIN(pipeline_), element));
}

void Pipeline::remove_vector(std::vector < GstElement * >&elementVec)
{
    std::vector < GstElement * >::iterator iter;
    for (iter = elementVec.begin(); iter != elementVec.end(); iter++)
        assert(gst_bin_remove(GST_BIN(pipeline_), *iter));
}

GstClock * Pipeline::clock() const
{
    return gst_pipeline_get_clock(GST_PIPELINE(pipeline_));
}
