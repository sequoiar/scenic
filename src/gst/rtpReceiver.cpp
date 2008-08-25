
// rtpReceiver.cpp
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

#include <cassert>
#include <iostream>
#include <list>
#include <algorithm>
#include <string.h>
#include <gst/gst.h>
#include "logWriter.h"
#include "gstLinkable.h"
#include "rtpReceiver.h"
#include "mediaConfig.h"

std::list<GstElement *> RtpReceiver::depayloaders_;


RtpReceiver::RtpReceiver()
    : rtp_receiver_(0), depayloader_(0)
{
    // empty
}


RtpReceiver::~RtpReceiver()
{
    assert(stop());
    pipeline_.remove(rtp_receiver_);

    // find this->depayloader in the static list of depayloaders
    std::list<GstElement *>::iterator iter;
    iter = std::find(depayloaders_.begin(), depayloaders_.end(), depayloader_);

    // make sure we found it and remove it
    assert(iter != depayloaders_.end());
    depayloaders_.erase(iter);
}


void RtpReceiver::set_caps(const char *capsStr)
{
    GstCaps *caps;
    assert(caps = gst_caps_from_string(capsStr));
    g_object_set(G_OBJECT(rtp_receiver_), "caps", caps, NULL);
    gst_caps_unref(caps);
}


void RtpReceiver::cb_new_src_pad(GstElement * , GstPad * srcPad, void *)
{
    // FIXME: Once this callback is attached to the pad-added signal, it gets called like crazy, any time any pad
    // is added (regardless of whether or not it's a dynamic pad) to rtpbin.
    if (gst_pad_is_linked(srcPad))
    {
        LOG("Pad is already linked", DEBUG)
        return;
    }
    else if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
    {
        LOG("Pad is not a source", DEBUG);
        return;
    }
    else if (strncmp(gst_pad_get_name(srcPad), "recv_rtp_src", 12))
    {
        LOG("Wrong pad", DEBUG);
        return;
    }
    // FIXME: We only have this really stupid method of comparing the caps strings of all
    // the sinks that have been attached to our RtpReceiver so far (stored in a list) against those of the new pad.
    GstPad *sinkPad = get_matching_sink_pad(srcPad);

    if (gst_pad_is_linked(sinkPad)) // only link once
    {
        LOG("sink pad is already linked.", WARNING);
        gst_object_unref(sinkPad);
        return;
    }
    assert(GstLinkable::link_pads(srcPad, sinkPad));

    gst_object_unref(sinkPad);
}


GstPad *RtpReceiver::get_matching_sink_pad(GstPad *srcPad)
{
    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(depayloaders_.front(), "sink");

    // look for caps whose first 37 characters match (this includes the parameter that describes media type)
    // FIXME: could just check the depayloader types/names

    const int CAPS_LEN = 37;
    std::list<GstElement *>::iterator iter = depayloaders_.begin();
    std::string srcCaps(gst_caps_to_string(gst_pad_get_caps(srcPad)));

    while (strncmp(gst_caps_to_string(gst_pad_get_caps(sinkPad)), srcCaps.c_str(), CAPS_LEN)
           && iter != depayloaders_.end())
    {
        gst_object_unref(sinkPad);
        sinkPad = gst_element_get_static_pad(*iter, "sink");
        ++iter;
    }

    return sinkPad;
}


void RtpReceiver::addDerived(GstElement * depayloader, const MediaConfig * config)
{
    GstPad *recv_rtp_sink;
    GstPad *send_rtcp_src;
    GstPad *recv_rtcp_sink;
    GstPad *rtpReceiverSrc;
    GstPad *rtcpReceiverSrc;
    GstPad *rtcpSenderSink;

    // store copy so that destructor knows which depayloader to remove from its list
    depayloader_ = depayloader;

    assert(rtp_receiver_ = gst_element_factory_make("udpsrc", NULL));
    g_object_set(rtp_receiver_, "port", config->port(), NULL);

    assert(rtcp_receiver_ = gst_element_factory_make("udpsrc", NULL));
    g_object_set(rtcp_receiver_, "port", config->port() + 1, NULL);

    assert(rtcp_sender_ = gst_element_factory_make("udpsink", NULL));
    g_object_set(rtcp_sender_, "host", config->remoteHost(), "port",
                 config->port() + 5, "sync", FALSE, "async", FALSE, NULL);

    pipeline_.add(rtp_receiver_);
    pipeline_.add(rtcp_receiver_);
    pipeline_.add(rtcp_sender_);

    assert(recv_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtp_sink_")));
    assert(send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_")));
    assert(recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_")));

    assert(rtpReceiverSrc = gst_element_get_static_pad(rtp_receiver_, "src"));
    assert(rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src"));
    assert(rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink"));

    assert(GstLinkable::link_pads(rtpReceiverSrc, recv_rtp_sink));
    assert(GstLinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink));
    assert(GstLinkable::link_pads(send_rtcp_src, rtcpSenderSink));

    depayloaders_.push_back(depayloader);
    // when pad is created, it must be linked to new sink
    g_signal_connect(rtpbin_, "pad-added", G_CALLBACK(RtpReceiver::cb_new_src_pad), NULL);

    // release request pads (in reverse order)
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    gst_element_release_request_pad(rtpbin_, recv_rtp_sink);

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtpReceiverSrc));
}


