
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


VideoReceiver::VideoReceiver(const VideoSession& session) : session_(session)
{
    // empty
}



VideoReceiver::~VideoReceiver() 
{
    // empty
}



bool VideoReceiver::init()
{
    //  Create receiver pipeline
    GstElement *rxSrc, *ffdec_h264, *rtph264depay, *rxSink;
    GstCaps *caps;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    rxSrc = gst_element_factory_make("udpsrc", "rxSrc");
    assert(rxSrc);

    rtph264depay = gst_element_factory_make("rtph264depay", "rtph264depay");
    assert(rtph264depay);

    ffdec_h264 = gst_element_factory_make("ffdec_h264", "ffdec_h264");
    assert(ffdec_h264);

    rxSink = gst_element_factory_make("xvimagesink", "rxSink");
    assert(rxSink);

    gst_bin_add_many(GST_BIN(pipeline_), rxSrc, rtph264depay, 
            ffdec_h264, rxSink, NULL); 

    caps = gst_caps_new_simple("application/x-rtp", NULL);
    assert(caps);

    g_object_set(G_OBJECT(rxSrc), "caps", caps, NULL);
    g_object_set(G_OBJECT(rxSrc), "port", session_.port(), NULL);
    g_object_set(G_OBJECT(rxSink), "sync", FALSE, NULL);

    std::cout << "Receiving media on port : " << session_.port() << std::endl;
    gst_element_link_many(rxSrc, rtph264depay, ffdec_h264, rxSink, NULL);

    return true;
}

