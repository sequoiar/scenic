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
#include "videoSource.h"
#include "videoConfig.h"
#include "logWriter.h"


VideoSource::VideoSource(const VideoConfig &config)
    : config_(config), source_(0)
{}

// parts of sub_init that are common to all AudioSource classes
void VideoSource::init()
{
    source_ = gst_element_factory_make(config_.source(), NULL);
    assert(source_);
    pipeline_.add(source_);

    sub_init();
}

void VideoSource::link_element(GstElement *sinkElement)
{
    assert(gst_element_link(source_, sinkElement));
}

VideoSource::~VideoSource()
{
    if (isPlaying())
        assert(pipeline_.stop());
    pipeline_.remove(source_);
}

// defers to subclassses callback
gboolean VideoSource::base_callback(GstClock *clock, GstClockTime time,GstClockID id,
                                    gpointer user_data)
{
    return (static_cast<VideoSource*>(user_data)->callback());
}

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
    const int BLACK = 2;    // gst-inspect property codes
    const int WHITE = 3;
    static int colour = BLACK;

    g_object_set(G_OBJECT(source_), "pattern", colour, NULL);
    colour = (colour == BLACK) ? WHITE : BLACK;     // toggle black and white
}

void VideoTestSource::sub_init()
{
    g_object_set(G_OBJECT(source_), "is-live", TRUE, NULL); // necessary for clocked callback to work

    // FIXME: move to pipeline class?
    clockId_ = gst_clock_new_periodic_id(pipeline_.clock()
                                         ,pipeline_.start_time(), GST_SECOND);
    gst_clock_id_wait_async(clockId_, base_callback, this);
}

VideoTestSource::~VideoTestSource()
{
    assert(pipeline_.stop());
    gst_clock_id_unschedule(clockId_);
    gst_clock_id_unref(clockId_);
}

void VideoFileSource::sub_init()
{
    decoder_ = gst_element_factory_make("decodebin", NULL);
    assert(decoder_);

    pipeline_.add(decoder_);

    const char *filename = "inputVid.avi";
    g_object_set(G_OBJECT(source_), "location", filename, NULL);
    assert(gst_element_link(source_, decoder_));
    g_signal_connect(decoder_, "new-decoded-pad"
                     ,G_CALLBACK(VideoFileSource::cb_new_src_pad), (void *)this);
}

void VideoFileSource::link_element(GstElement *sinkElement)
{
    // defer linking of decoder to this element to callback
    sinkElement_ = sinkElement;
    if (!strncmp(gst_element_get_name(sinkElement_), "xvimagesink"
                 ,strlen("xvimagesink")))
        g_object_set(G_OBJECT(sinkElement_), "sync", TRUE, NULL);
}

void VideoFileSource::cb_new_src_pad(GstElement * srcElement, GstPad * srcPad,
                                     gboolean last,
                                     void *data)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG("Pad is already linked.")
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

    LOG("VideoFileSource: linking new srcpad and sinkpad.");
    assert(link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}

VideoFileSource::~VideoFileSource()
{
    assert(pipeline_.stop());
    pipeline_.remove(decoder_);
}

VideoDvSource::VideoDvSource(const VideoConfig &config)
    : VideoSource(config)
    ,demux_(0), queue_(0), dvdec_(0)
{}

void VideoDvSource::link_element(GstElement *sinkElement)
{
    assert(gst_element_link(dvdec_, sinkElement));
}

void VideoDvSource::sub_init()
{
    demux_ = gst_element_factory_make("dvdemux", NULL);
    assert(demux_);
    queue_ = gst_element_factory_make("queue", NULL);
    assert(queue_);
    dvdec_ = gst_element_factory_make("dvdec", NULL);
    assert(dvdec_);

    // demux has dynamic pads
    pipeline_.add(demux_);
    pipeline_.add(queue_);
    pipeline_.add(dvdec_);

    // demux srcpad must be linked to queue sink pad at runtime
    g_signal_connect(demux_, "pad-added", G_CALLBACK(VideoDvSource::cb_new_src_pad)
                     ,(void *) queue_);

    assert(gst_element_link(source_, demux_));
    assert(gst_element_link(queue_, dvdec_));
}

void VideoDvSource::cb_new_src_pad(GstElement * srcElement, GstPad * srcPad,
                                   void *data)
{
    // ignore audio from dvsrc
    if (!std::string("audio").compare(gst_pad_get_name(srcPad)))
    {
        LOG("Ignoring audio stream from DV");
        return;
    }
    GstElement *sinkElement = (GstElement *) data;
    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(sinkElement, "sink");
    LOG("VideoDvSource: linking new srcpad and sinkpad.");
    assert(link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}

