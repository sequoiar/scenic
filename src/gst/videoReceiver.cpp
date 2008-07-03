
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

VideoReceiver::VideoReceiver(const VideoConfig & config):MediaBase(dynamic_cast < const MediaConfig & >(config)),
config_(config), decoder_(0), depayloader_(0), sink_(0)
{
    // empty
}

VideoReceiver::~VideoReceiver()
{
    assert(stop());

    pipeline_.remove(decoder_);
    pipeline_.remove(depayloader_);
    pipeline_.remove(sink_);
}

void VideoReceiver::init_source()
{
}

void VideoReceiver::init_codec()
{
    if (config_.has_h264())
    {
        depayloader_ = gst_element_factory_make("rtph264depay", NULL);
        assert(depayloader_);

        decoder_ = gst_element_factory_make("ffdec_h264", NULL);
        assert(decoder_);

    }

    pipeline_.add(depayloader_);
    pipeline_.add(decoder_);
    assert(gst_element_link_many(depayloader_, decoder_, NULL));

    session_.add(depayloader_, dynamic_cast <const MediaConfig &>(config_));
    session_.set_caps("application/x-rtp");
}

void VideoReceiver::init_sink()
{
    sink_ = gst_element_factory_make("xvimagesink", NULL);
    assert(sink_);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

    pipeline_.add(sink_);
    assert(gst_element_link(decoder_, sink_));
}

