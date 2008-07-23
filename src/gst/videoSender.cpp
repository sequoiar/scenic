
// videoSender.cpp
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
#include <iostream>
#include <string>
#include <cassert>

#include "videoSender.h"
#include "videoSource.h"
#include "logWriter.h"

// FIXME: pull out videoSource stuff into videoSource class hierarchy

VideoSender::VideoSender(const VideoConfig & config) :
    config_(config), 
    session_(), 
    source_(0), colorspc_(0), 
    encoder_(0), payloader_(0), sink_(0)
{
    // empty
}



VideoSender::~VideoSender()
{
    assert(stop());
    pipeline_.remove(sink_);
    pipeline_.remove(payloader_);
    pipeline_.remove(encoder_);
    pipeline_.remove(colorspc_);
    delete source_;
}



void VideoSender::init_source()
{
    source_ = config_.createSource();
    assert(source_);
    source_->init();
}



void VideoSender::init_codec()
{
    if (config_.has_h264()) {
        colorspc_ = gst_element_factory_make("ffmpegcolorspace", NULL);
        assert(colorspc_);
        pipeline_.add(colorspc_);
        encoder_ = gst_element_factory_make("x264enc", NULL);
        assert(encoder_);
        g_object_set(G_OBJECT(encoder_), "bitrate", 2048, "byte-stream", TRUE, "threads", 4, NULL);
        pipeline_.add(encoder_);
        assert(gst_element_link_many(source_->output(), colorspc_, encoder_, NULL));
    }
}



void VideoSender::init_sink()
{
    if (config_.isNetworked()) {
        payloader_ = gst_element_factory_make("rtph264pay", NULL);
        assert(payloader_);
        pipeline_.add(payloader_);
        assert(gst_element_link(encoder_, payloader_));
        session_.add(payloader_, &config_);
    }
    else                        // local test only, no encoding
    {
        sink_ = gst_element_factory_make("xvimagesink", NULL);
        g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
        pipeline_.add(sink_);
        assert(gst_element_link(source_->output(), sink_));
    }
}

