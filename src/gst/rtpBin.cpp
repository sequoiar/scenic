
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
#include <cstring>
#include <sstream>

#include "rtpBin.h"
#include "rtpPay.h"
#include "remoteConfig.h"
#include "pipeline.h"
#include "logWriter.h"


GstElement *RtpBin::rtpbin_ = 0;
unsigned int RtpBin::refCount_ = 0;


void RtpBin::init()
{
    // only initialize rtpbin once per process
    if (!rtpbin_) {
        rtpbin_ = gst_element_factory_make("gstrtpbin", NULL);
        assert(rtpbin_);
        // g_object_set(G_OBJECT(rtpbin_), "latency", 10, NULL); // has no visible impact
        // FIXME
#if 0  
        this is probably not useful
        // will send an event downstraem when a packet is lost
        GValue doLost;
        memset(&doLost, 0, sizeof(doLost));
        g_value_init(&doLost, G_TYPE_BOOLEAN);
        g_value_set_boolean(&doLost, TRUE);
        gst_child_proxy_set_property(GST_OBJECT(rtpbin_), "do-lost", &doLost);
#endif
        pipeline_.add(rtpbin_);
    }
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
    stop();
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


double RtpBin::bandwidth() const
{
    double result = 0.0;

    // FIXME: use gstrtpbin's get internal rtpsession method
    GstIteratorResult iterResult;
    GstElement *elem;
    GstIterator *iter;
    iter = gst_bin_iterate_recurse(GST_BIN(rtpbin_));
    iterResult = gst_iterator_next(iter, (void**) &elem);
    const static char *CHILD_NAME = "rtpsession";
    const static short NAME_LENGTH = strlen(CHILD_NAME);

    while (iterResult != GST_ITERATOR_DONE)
    {
        if (strncmp(CHILD_NAME, gst_element_get_name(elem), NAME_LENGTH) == 0)
        {
            g_object_get(G_OBJECT(elem), "bandwidth", &result, NULL);
            gst_object_unref(elem);
            break;
        }
        gst_object_unref(elem);
        iterResult = gst_iterator_next(iter, (void **) &elem);
    }
    LOG_DEBUG("BANDWITH USED = " << result);

    return result;
}

