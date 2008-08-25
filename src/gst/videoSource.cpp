// videoSource.cpp
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
#include <string>
#include <string.h>
#include <cassert>
#include "gstLinkable.h"
#include "videoSource.h"
#include "videoConfig.h"
#include "logWriter.h"


VideoSource::VideoSource(const VideoConfig &config)
    : config_(config), source_(0)
{}

// parts of sub_init that are common to all VideoSource classes
void VideoSource::init()
{
    assert(source_ = gst_element_factory_make(config_.source(), NULL));
    pipeline_.add(source_);

    sub_init();
}


void VideoSource::link_element(GstElement *sinkElement)
{
    GstLinkable::link(source_, sinkElement);
}


VideoSource::~VideoSource()
{
    assert(stop());
    pipeline_.remove(source_);
}


// defers to subclassses callback
gboolean VideoSource::base_callback(GstClock * /*clock*/, GstClockTime  /*time*/, GstClockID  /*id*/,
                                    gpointer user_data)
{
    VideoSource* context = static_cast<VideoSource*>(user_data);
    return context->callback();
}


// gst-inspect property codes
const int VideoTestSource::BLACK = 2;
const int VideoTestSource::WHITE = 3;

// toggle colour
gboolean VideoTestSource::callback()
{
    if (!source_)
        return FALSE;
    toggle_colour();
    return TRUE;
}


void VideoTestSource::toggle_colour()
{
    static int colour = BLACK;

    g_object_set(G_OBJECT(source_), "pattern", colour, NULL);
    colour = (colour == BLACK) ? WHITE : BLACK;     // toggle black and white
}


void VideoTestSource::sub_init()
{
    g_object_set(G_OBJECT(source_), "is-live", TRUE, NULL); // necessary for clocked callback to work
    g_object_set(G_OBJECT(source_), "pattern", WHITE, NULL);

    clockId_ = pipeline_.add_clock_callback(base_callback, this);
}


VideoTestSource::~VideoTestSource()
{
    assert(stop());
    pipeline_.remove_clock_callback(clockId_);
}


void VideoFileSource::sub_init()
{
    assert(decoder_ = gst_element_factory_make("decodebin", NULL));

    pipeline_.add(decoder_);

    g_object_set(G_OBJECT(source_), "location", config_.location(), NULL);
    GstLinkable::link(source_, decoder_);

    // bind callback
    g_signal_connect(decoder_, "new-decoded-pad",
                     G_CALLBACK(VideoFileSource::cb_new_src_pad),
                     static_cast<void *>(this));
}


void VideoFileSource::link_element(GstElement *sinkElement)
{
    // defer linking of decoder to this element to callback
    sinkElement_ = sinkElement;

    // set xvimagesink to true for filesrc to work properly
    if (!strncmp(gst_element_get_name(sinkElement_), "xvimagesink", strlen("xvimagesink")))
        g_object_set(G_OBJECT(sinkElement_), "sync", TRUE, NULL);
}


void VideoFileSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, gboolean  /*last*/,
                                     void *data)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG("Pad is already linked.", DEBUG)
        return;
    }

    VideoFileSource *context = static_cast<VideoFileSource*>(data);
    GstStructure *str;
    GstPad *sinkPad;
    GstCaps *caps;

    sinkPad = gst_element_get_static_pad(context->sinkElement_, "sink");
    if (GST_PAD_IS_LINKED(sinkPad))
    {
        g_object_unref(sinkPad);        // don't link more than once
        return;
    }
    /* check media type */
    caps = gst_pad_get_caps(srcPad);
    str = gst_caps_get_structure(caps, 0);
    if (!g_strrstr(gst_structure_get_name(str), "video"))
    {
        gst_caps_unref(caps);
        gst_object_unref(sinkPad);
        return;
    }
    gst_caps_unref(caps);

    LOG("VideoFileSource: linking new srcpad and sinkpad.", DEBUG);
    assert(GstLinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


VideoFileSource::~VideoFileSource()
{
    assert(stop());
    pipeline_.remove(decoder_);
}


VideoDvSource::VideoDvSource(const VideoConfig &config)
    : VideoSource(config), demux_(0), queue_(0), dvdec_(0)
{}


void VideoDvSource::link_element(GstElement *sinkElement)
{
    GstLinkable::link(dvdec_, sinkElement);
}


void VideoDvSource::sub_init()
{
    assert(demux_ = gst_element_factory_make("dvdemux", NULL));
    assert(queue_ = gst_element_factory_make("queue", NULL));
    assert(dvdec_ = gst_element_factory_make("dvdec", NULL));

    // demux has dynamic pads
    pipeline_.add(demux_);
    pipeline_.add(queue_);
    pipeline_.add(dvdec_);

    // demux srcpad must be linked to queue sink pad at runtime
    g_signal_connect(demux_, "pad-added",
                     G_CALLBACK(VideoDvSource::cb_new_src_pad),
                     static_cast<void *>(queue_));

    GstLinkable::link(source_, demux_);
    GstLinkable::link(queue_, dvdec_);
}


void VideoDvSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, void *data)
{
    if (std::string("audio") == gst_pad_get_name(srcPad))
    {
        LOG("Ignoring audio stream from DV", DEBUG);
        return;
    }
    GstElement *sinkElement = static_cast<GstElement *>(data);
    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(sinkElement, "sink");

    if (GST_PAD_IS_LINKED(sinkPad))
    {
        g_object_unref(sinkPad);        // don't link more than once
        return;
    }
    LOG("VideoDvSource: linking new srcpad to sinkpad.", DEBUG);
    assert(GstLinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


