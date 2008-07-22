
// rtpSession.cpp
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
#include <sstream>

#include "rtpSession.h"
#include "mediaConfig.h"
#include "logWriter.h"

GstElement *RtpSession::rtpbin_ = 0;
int RtpSession::refCount_ = 0;

RtpSession::RtpSession() : rtcp_sender_(0), rtcp_receiver_(0)
{
    ++refCount_;
}

bool RtpSession::init()
{
    // only initialize rtpbin once per process
    if (!rtpbin_) {
        rtpbin_ = gst_element_factory_make("gstrtpbin", NULL);
        assert(rtpbin_);

        pipeline_.add(rtpbin_);
    }
    return true;
}

void RtpSession::add(GstElement * elem, const MediaConfig * config)
{
    RtpSession::init();

    rtcp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtcp_sender_);
    g_object_set(rtcp_sender_, "host", config->remoteHost(), "port", config->port() + 1, "sync",
                 FALSE, "async", FALSE, NULL);

    rtcp_receiver_ = gst_element_factory_make("udpsrc", NULL);
    assert(rtcp_receiver_);
    g_object_set(rtcp_receiver_, "port", config->port() + 5, NULL);

    pipeline_.add(rtcp_sender_);
    pipeline_.add(rtcp_receiver_);

    addDerived(elem, config);
}

const char *RtpSession::padStr(const char *padName)
{
    std::string result(padName);
    std::stringstream istream;

    istream << refCount_ - 1;        // 0-based
    result = result + istream.str();
    return result.c_str();
}

RtpSession::~RtpSession()
{
    assert(pipeline_.stop());

    pipeline_.remove(rtcp_sender_);
    pipeline_.remove(rtcp_receiver_);

    --refCount_;
    if (refCount_ <= 0) 
    {
        assert(refCount_ == 0);
        pipeline_.remove(rtpbin_);
        rtpbin_ = 0;
    }
}

