/* videoSource.cpp
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

#include "gstLinkable.h"
#include "videoSource.h"
#include "pipeline.h"
#include "videoConfig.h"

#include "dv1394.h"

/// Constructor
VideoSource::VideoSource(const VideoSourceConfig &config) : 
    config_(config), source_(0) 
{}


/// Callable by child classes
void VideoSource::init()
{
    source_ = Pipeline::Instance()->makeElement(config_.source(), NULL);
}


/// Destructor
VideoSource::~VideoSource()
{
    Pipeline::Instance()->remove(&source_);
}


/// Constructor
VideoTestSource::VideoTestSource(const VideoSourceConfig &config) : 
    VideoSource(config),
    capsFilter_(0),
    width_(768),
    height_(480)
{}

void VideoTestSource::init()
{
    VideoSource::init();
    g_object_set(G_OBJECT(source_), "is-live", FALSE, NULL); // necessary for clocked callback to work
    
    capsFilter_ = Pipeline::Instance()->makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);
}


// optionally called to force resolution
void VideoTestSource::filterCaps()
{
    // otherwise videotestsrc defaults to 320 by 240
    std::ostringstream capsStr;
    capsStr << "video/x-raw-yuv, width=" << width_ << ", height=" << height_; 

    GstCaps *videoCaps = gst_caps_from_string(capsStr.str().c_str());
    g_object_set(G_OBJECT(capsFilter_), "caps", videoCaps, NULL);

    gst_caps_unref(videoCaps);
    
}


/// Destructor
VideoTestSource::~VideoTestSource()
{
    Pipeline::Instance()->remove(&capsFilter_);
}


/// Constructor
VideoFileSource::VideoFileSource(const VideoSourceConfig &config) : 
    VideoSource(config), decoder_(0) 
{}

void VideoFileSource::init()
{
    VideoSource::init();
    decoder_ = Pipeline::Instance()->makeElement("decodebin", NULL);

    assert(config_.fileExists());
    g_object_set(G_OBJECT(source_), "location", config_.location(), NULL);
    gstlinkable::link(source_, decoder_);

    // bind callback
    g_signal_connect(decoder_, "new-decoded-pad",
            G_CALLBACK(VideoFileSource::cb_new_src_pad),
            static_cast<void *>(this));
}


void VideoFileSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, gboolean /*last*/, void * /*data*/)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG_DEBUG("Pad is already linked.");
        return;
    }
    GstStructure *str;
    GstPad *sinkPad;
    GstCaps *caps;
    GstElement *sinkElement;

    // FIXME: HACK ATTACK!!!!
    // looks for element named colorspc in pipeline, which 
    // will be the case for VideoSender. for VideoLocal, 
    // it will look for videosink. 
    sinkElement = Pipeline::Instance()->findElement("colorspc");

    if (!sinkElement)       // we're local!
    {
        sinkElement = Pipeline::Instance()->findElement("videosink");
        g_object_set(G_OBJECT(sinkElement), "sync", TRUE, NULL);
    }
    
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


/// Destructor
VideoFileSource::~VideoFileSource()
{
    Pipeline::Instance()->remove(&decoder_);
}


/// Constructor
VideoDvSource::VideoDvSource(const VideoSourceConfig &config) : 
    VideoSource(config), queue_(0), dvdec_(0)
{}


/// Destructor
VideoDvSource::~VideoDvSource()
{
    Pipeline::Instance()->remove(&queue_);
    Pipeline::Instance()->remove(&dvdec_);
    Dv1394::Instance()->unsetVideoSink();
}


void VideoDvSource::init()
{
    queue_ = Pipeline::Instance()->makeElement("queue", NULL);
    dvdec_ = Pipeline::Instance()->makeElement("dvdec", NULL);

    Dv1394::Instance()->setVideoSink(queue_);
    gstlinkable::link(queue_, dvdec_);
}


void VideoV4lSource::init()
{
    VideoSource::init();
    // set a v4l2src if given to config as an arg, otherwise use default
    if (config_.hasLocation() && config_.fileExists())
        g_object_set(G_OBJECT(source_), "device", config_.location(), NULL);
    
    capsFilter_ = Pipeline::Instance()->makeElement("capsfilter", NULL);
    gstlinkable::link(source_, capsFilter_);

    // FIXES BUG WITH IMAGE BEING DISTORTED
    std::ostringstream capsStr;
    capsStr << "video/x-raw-yuv, pixel-aspect-ratio=10/11"; 

    GstCaps *videoCaps = gst_caps_from_string(capsStr.str().c_str());
    g_object_set(G_OBJECT(capsFilter_), "caps", videoCaps, NULL);

    gst_caps_unref(videoCaps);
}

