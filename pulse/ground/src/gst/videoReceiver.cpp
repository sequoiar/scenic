
// videoReceiver.cpp
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


#include <iostream>
#include <cassert>
#include <gst/gst.h>

#include "mediaBase.h"
#include "videoReceiver.h"


VideoReceiver::VideoReceiver(const VideoConfig& config) : MediaBase(dynamic_cast<const MediaConfig&>(config)), config_(config)
{
    // empty
}



VideoReceiver::~VideoReceiver() 
{
    assert(stop());

    pipeline_.remove(src_);
    pipeline_.remove(decoder_);
    pipeline_.remove(depayloader_);
    pipeline_.remove(sink_);
}



void VideoReceiver::init_source()
{
    GstCaps *caps;
    caps = gst_caps_new_simple("application/x-rtp", NULL);
    assert(caps);

    src_ = gst_element_factory_make("udpsrc", NULL);
    assert(src_);

    g_object_set(G_OBJECT(src_), "caps", caps, NULL);
    g_object_set(G_OBJECT(src_), "port", config_.port(), NULL);
    
    pipeline_.add(src_);
    gst_caps_unref(caps);
}



void VideoReceiver::init_codec()
{
    if (config_.has_h264())
    {
        depayloader_ = gst_element_factory_make("rtph264depay", NULL);
        assert(depayloader_);

        decoder_ = gst_element_factory_make("ffdec_h264", NULL);
        assert(decoder_);

    //    g_object_set(G_OBJECT(decoder_), "debug-mv", TRUE, NULL);

        pipeline_.add(depayloader_);
        pipeline_.add(decoder_);
        assert(gst_element_link_many(src_, depayloader_, decoder_, NULL));
    }
}



void VideoReceiver::init_sink()
{
    sink_ = gst_element_factory_make("xvimagesink", NULL);
    assert(sink_);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

    pipeline_.add(sink_);
    assert(gst_element_link(decoder_, sink_));
}

