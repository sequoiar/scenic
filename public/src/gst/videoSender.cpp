
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
#include <cassert>

#include "gstLinkable.h"
#include "videoSender.h"
#include "videoSource.h"
#include "videoConfig.h"
#include "videoSink.h"


VideoSender::~VideoSender()
{
    assert(stop());
    pipeline_.remove(&payloader_);
    pipeline_.remove(&encoder_);
    pipeline_.remove(&colorspc_);
    delete source_;
}


void VideoSender::init_source()
{
    assert(source_ = config_.createSource());
    source_->init();
}


void VideoSender::init_codec()
{
    if (config_.has_h264()) {
        assert(colorspc_ = gst_element_factory_make("ffmpegcolorspace", "colorspc"));
        pipeline_.add(colorspc_);

        assert(encoder_ = gst_element_factory_make("x264enc", NULL));
        g_object_set(G_OBJECT(encoder_), "bitrate", 2048, "byte-stream", TRUE, "threads", 4,
                     NULL);
        pipeline_.add(encoder_);

        GstLinkable::link(*source_, colorspc_);
        GstLinkable::link(colorspc_, encoder_);
    }
}


void VideoSender::init_sink()
{
    if (config_.isNetworked()) {
        assert(payloader_ = gst_element_factory_make("rtph264pay", NULL));
        pipeline_.add(payloader_);
        GstLinkable::link(encoder_, payloader_);
        session_.add(payloader_, config_);
    }
    else {                 // local test only, no encoding
        sink_.init();
        GstLinkable::link(*source_, sink_);   // FIXME: this shouldn't happen for VideoFileSource
    }
}


bool VideoSender::start()
{
    GstBase::start();
    pipeline_.wait_until_playing(); // otherwise it doesn't know it's playing
    if (!config_.isNetworked())
        sink_.showWindow();
    return true;
}


