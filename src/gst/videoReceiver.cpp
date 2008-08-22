
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

#include "lo/lo.h"

#include "mediaBase.h"
#include "gstLinkable.h"
#include "videoReceiver.h"
#include "logWriter.h"

VideoReceiver::VideoReceiver(const VideoConfig & config)
    : config_(config), session_(), depayloader_(0), decoder_(0), sink_(0)
{
    // empty
}


VideoReceiver::~VideoReceiver()
{
    assert(stop());
    pipeline_.remove(sink_);
    pipeline_.remove(decoder_);
    pipeline_.remove(depayloader_);
}


void VideoReceiver::init_codec()
{
    if (config_.has_h264()) {
        assert(depayloader_ = gst_element_factory_make("rtph264depay", NULL));
        assert(decoder_ = gst_element_factory_make("ffdec_h264", NULL));
    }
    pipeline_.add(depayloader_);
    pipeline_.add(decoder_);
    GstLinkable::link(depayloader_, decoder_);

    session_.add(depayloader_, &config_);
    session_.set_caps("application/x-rtp,media=(string)video,clock-rate=(int)90000,"
                      "encoding-name=(string)H264");
}


void VideoReceiver::init_sink()
{
    assert(sink_ = gst_element_factory_make("xvimagesink", NULL));
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

    pipeline_.add(sink_);
    GstLinkable::link(decoder_, sink_);
}


bool VideoReceiver::start()
{
    std::cout << "Receiving video on port " << config_.port() << std::endl;
    MediaBase::start();
    return true;
}


bool VideoReceiver::stop()
{
    MediaBase::stop();
    stop_sender();
    return true;
}


void VideoReceiver::stop_sender() const
{
    LOG("Telling sender to stop...", DEBUG);

    lo_address t = lo_address_new(NULL, "8880");
    if (lo_send(t, "/video/tx/stop", NULL) == -1)
    {
        std::cerr << "OSC error " << lo_address_errno(t) << ": " <<
        lo_address_errstr(t) <<
        std::endl;
        exit(EXIT_FAILURE);
    }
}


