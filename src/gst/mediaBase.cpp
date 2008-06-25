
// mediaBase.cpp
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

#include "mediaBase.h"
#include "logWriter.h"

bool MediaBase::gstInitialized_ = false;

MediaBase::MediaBase() : pipeline_(0), verbose_(false)
{
    if (!gstInitialized_)
    {
        gstInitialized_ = true;
        // should only be called once in a process
        gst_init(0, NULL);
    }
}



MediaBase::~MediaBase()
{
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}



bool MediaBase::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    return true;
}



bool MediaBase::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    return !isPlaying();
}



bool MediaBase::isPlaying() const 
{ 
    if (pipeline_ && (GST_STATE(pipeline_) == GST_STATE_PLAYING))
        return true; 
    else
        return false;
}



void MediaBase::init_pipeline()
{
    pipeline_ = gst_pipeline_new("pipeline");
    assert(pipeline_);

    if (verbose_)
        make_verbose();
}


void MediaBase::make_verbose()
{
    // Get verbose output
    if (verbose_) 
    {
        gchar *exclude_args = NULL; // set args to be excluded from output
        gchar **exclude_list =
            exclude_args ? g_strsplit (exclude_args, ",", 0) : NULL;
        g_signal_connect (pipeline_, "deep_notify",
                G_CALLBACK (gst_object_default_deep_notify), exclude_list);
    }
}



void MediaBase::wait_until_playing()
{
    while (!isPlaying())  // wait for pipeline to get rolling
        usleep(1000);
}



void MediaBase::cb_new_src_pad(GstElement *srcElement, GstPad *srcPad, void * data)
{
    GstElement *sinkElement = (GstElement *) data;
    GstPad *sinkPad;
    LOG("Dynamic pad created, linking new srcpad and sinkpad.");
    
    sinkPad = gst_element_get_static_pad(sinkElement, "sink");
    assert(gst_pad_link(srcPad, sinkPad) == GST_PAD_LINK_OK);
    gst_object_unref(sinkPad);
}



bool MediaBase::init()
{
    init_pipeline();
    // these methods are defined in subclasses
    init_source();
    init_codec();
    init_sink();
}
