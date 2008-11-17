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

#include <string>
#include <cassert>
#include <fstream>
#include <gst/gst.h>
#include "raw1394Util.h"
#include "gstLinkable.h"
#include "videoSource.h"
#include "pipeline.h"
#include "videoConfig.h"
#include "logWriter.h"


// parts of sub_init that are common to all VideoSource classes
void VideoSource::init()
{
    assert(source_ = gst_element_factory_make(config_.source(), NULL));
    pipeline_.add(source_);

    sub_init();
}


VideoSource::~VideoSource()
{
    stop();
    pipeline_.remove(&source_);
}


// defers to subclassses callback
gboolean VideoSource::base_callback(GstClock * /*clock*/, GstClockTime /*time*/, GstClockID /*id*/,
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
    g_object_set(G_OBJECT(source_), "is-live", FALSE, NULL); // necessary for clocked callback to work
    //g_object_set(G_OBJECT(source_), "pattern", WHITE, NULL);

    //clockId_ = pipeline_.add_clock_callback(base_callback, this);
}


VideoTestSource::~VideoTestSource()
{
    stop();
    pipeline_.remove_clock_callback(clockId_);
}


void VideoFileSource::sub_init()
{
    assert(decoder_ = gst_element_factory_make("decodebin", NULL));

    pipeline_.add(decoder_);

    assert(config_.fileExists());
    g_object_set(G_OBJECT(source_), "location", config_.location(), NULL);
    gstlinkable::link(source_, decoder_);
    
    // bind callback
    g_signal_connect(decoder_, "new-decoded-pad",
                     G_CALLBACK(VideoFileSource::cb_new_src_pad),
                     static_cast<void *>(this));
}


void VideoFileSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, gboolean /*last*/, void *data)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG_DEBUG("Pad is already linked.");
        return;
    }
    VideoFileSource *context = static_cast<VideoFileSource*>(data);
    GstStructure *str;
    GstPad *sinkPad;
    GstCaps *caps;
    GstElement *sinkElement;

    //sinkPad = gst_element_get_static_pad(context->sinkElement_, "sink");
    // FIXME: HACK!!!!
    //if (context->config_.isNetworked())
    sinkElement = context->pipeline_.findElement("colorspc");
    
    if (!sinkElement)       // we're local!
    {
        sinkElement = context->pipeline_.findElement("videosink");
        g_object_set(G_OBJECT(sinkElement), "sync", TRUE, NULL);
    }
    //#endif
    //FIXME: this assumes that the first pad it finds is indeed the one filesource
    // should connect to...maybe should just have its own ghost pad?
    //   sinkPad = context->pipeline_.findUnconnectedSinkpad();
    assert(sinkElement);
    sinkPad = gst_element_get_static_pad(sinkElement, "sink");

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

    LOG_DEBUG("VideoFileSource: linking new srcpad and sinkpad.");
    assert(gstlinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


VideoFileSource::~VideoFileSource()
{
    stop();
    pipeline_.remove(&decoder_);
}


VideoDvSource::~VideoDvSource()
{
    stop();
    if (pipeline_.findElement(config_.source()) != NULL)
        pipeline_.remove(&source_);
    source_ = NULL;
}


void VideoDvSource::init()
{
    if (!Raw1394::cameraIsReady())
        THROW_ERROR("Camera is not ready");

    source_ = pipeline_.findElement(config_.source());
    dvIsNew_ = source_ == NULL;
    if (dvIsNew_)
    {
        assert(source_ = gst_element_factory_make(config_.source(), config_.source()));
        pipeline_.add(source_);
    }
    sub_init();
}


void VideoDvSource::sub_init()
{
    demux_ = pipeline_.findElement("dvdemux");
    dvIsNew_ = demux_ == NULL;
    if (dvIsNew_)
    {
        assert(demux_ = gst_element_factory_make("dvdemux", "dvdemux"));
        // demux has dynamic pads
        pipeline_.add(demux_);
    }
    else
        assert(demux_);
    assert(queue_ = gst_element_factory_make("queue", NULL));
    assert(dvdec_ = gst_element_factory_make("dvdec", NULL));

    pipeline_.add(queue_);
    pipeline_.add(dvdec_);

    // demux srcpad must be linked to queue sink pad at runtime
    g_signal_connect(demux_, "pad-added",
            G_CALLBACK(VideoDvSource::cb_new_src_pad),
            static_cast<void *>(queue_));

    if (dvIsNew_)
        gstlinkable::link(source_, demux_);
    gstlinkable::link(queue_, dvdec_);
}


void VideoDvSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, void *data)
{
    if (std::string("audio") == gst_pad_get_name(srcPad))
    {
        LOG_DEBUG("Ignoring audio stream from DV");
        return;
    }
    else if (std::string("video") == gst_pad_get_name(srcPad))
    {
        LOG_DEBUG("Got video stream from DV");
    }
    else{
        LOG_DEBUG("Ignoring unknown stream from DV");
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
    LOG_DEBUG("VideoDvSource: linking new srcpad to sinkpad.");
    assert(gstlinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}

void VideoV4lSource::sub_init()
{
    // set a v4l2src if given to config as an arg, otherwise use default
    if (config_.hasLocation() && config_.fileExists())
        g_object_set(G_OBJECT(source_), "device", config_.location(), NULL);
}

