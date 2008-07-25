
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

#include "rtpSender.h"
#include "mediaConfig.h"
#include "logWriter.h"



RtpSender::RtpSender() : rtp_sender_(0)
{
}



RtpSender::~RtpSender()
{
    if (isPlaying())
        assert(pipeline_.stop());
    pipeline_.remove(rtp_sender_);
}



const char *RtpSender::caps_str() const
{
    assert(pipeline_.isPlaying());

    GstPad *pad;
    GstCaps *caps;

    pad = gst_element_get_pad(GST_ELEMENT(rtp_sender_), "sink");
    assert(pad);

    do
        caps = gst_pad_get_negotiated_caps(pad);
    while (caps == NULL);
    assert(caps != NULL);

    gst_object_unref(pad);

    const char *result = gst_caps_to_string(caps);
    gst_caps_unref(caps);
    return result;
}



void RtpSender::addDerived(GstElement * newSrc, const MediaConfig * config)
{
    GstPad *send_rtp_sink, *send_rtp_src, *send_rtcp_src, *recv_rtcp_sink, *payloadSrc,
           *rtpSenderSink, *rtcpSenderSink, *rtcpReceiverSrc;

    rtp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtp_sender_);
    g_object_set(rtp_sender_, "host", config->remoteHost(), "port", config->port(), "sync",
            FALSE, "async", FALSE, NULL);
    pipeline_.add(rtp_sender_);

    send_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("send_rtp_sink_"));
    assert(send_rtp_sink);

    send_rtp_src = gst_element_get_static_pad(rtpbin_, padStr("send_rtp_src_"));
    assert(send_rtp_src);

    send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_"));
    assert(send_rtcp_src);

    recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_"));
    assert(recv_rtcp_sink);

    payloadSrc = gst_element_get_static_pad(newSrc, "src");
    assert(payloadSrc);
    rtpSenderSink = gst_element_get_static_pad(rtp_sender_, "sink");
    assert(rtpSenderSink);
    rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink");
    assert(rtcpSenderSink);
    rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src");
    assert(rtcpReceiverSrc);

    // link pads
    assert(gst_pad_link(payloadSrc, send_rtp_sink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(send_rtp_src, rtpSenderSink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(send_rtcp_src, rtcpSenderSink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(rtcpReceiverSrc, recv_rtcp_sink) == GST_PAD_LINK_OK);

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtpSenderSink));
    gst_object_unref(GST_OBJECT(payloadSrc));

    // release request and static pads (in reverse order)
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    gst_object_unref(GST_OBJECT(send_rtp_src));
    gst_element_release_request_pad(rtpbin_, send_rtp_sink);
}

