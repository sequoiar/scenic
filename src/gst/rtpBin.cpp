
// rtpBin.cpp
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

#include "rtpBin.h"
#include "rtpPay.h"
#include "remoteConfig.h"
#include "pipeline.h"


GstElement *RtpBin::rtpbin_ = 0;
int RtpBin::refCount_ = 0;


bool RtpBin::init()
{
    // only initialize rtpbin once per process
    if (!rtpbin_) {
        rtpbin_ = gst_element_factory_make("gstrtpbin", NULL);
        assert(rtpbin_);

        pipeline_.add(rtpbin_);
    }
    return true;
}


const char *RtpBin::padStr(const char *padName)
{
    std::string result(padName);
    std::stringstream istream;

    istream << refCount_ - 1;        // 0-based
    result = result + istream.str();
    return result.c_str();
}


RtpBin::~RtpBin()
{
    assert(stop());
    pipeline_.remove(&rtcp_sender_);
    pipeline_.remove(&rtcp_receiver_);

    --refCount_;
    if (refCount_ <= 0) // destroy if no streams are present
    {
        assert(refCount_ == 0);
        pipeline_.remove(&rtpbin_);
        rtpbin_ = 0;
    }
}


