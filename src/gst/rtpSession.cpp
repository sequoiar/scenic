
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
#include "mediaBase.h"
#include "logWriter.h"

GstElement *RtpSession::rtpbin_ = 0;
int RtpSession::instanceCount_ = -1;

RtpSession::RtpSession() : rtcp_sender_(0), rtcp_receiver_(0)
{
    instanceCount_++;
}

bool RtpSession::init()
{
    // only initialize rtpbin once per process
    if (!rtpbin_)
    {
        rtpbin_ = gst_element_factory_make("gstrtpbin", NULL);
        assert(rtpbin_);

        pipeline_.add(rtpbin_);
    }
    return true;
}

void RtpSession::add(GstElement * elem, const MediaConfig & config)
{
    RtpSession::init();
    addDerived(elem, config);
}

const char *RtpSession::padStr(const char *padName)
{
    std::string result(padName);
    std::stringstream istream;

    istream << instanceCount_;        // 0-based
    result = result + istream.str();
    return result.c_str();
}

RtpSession::~RtpSession()
{
    assert(pipeline_.stop());
    pipeline_.remove(rtcp_sender_);
    pipeline_.remove(rtcp_receiver_);
    if (--instanceCount_ < 0)
    {
        pipeline_.remove(rtpbin_);
        rtpbin_ = 0;
        assert(instanceCount_ == -1);
    }
}

RtpSender::RtpSender() : rtp_sender_(0)
{
}

RtpSender::~RtpSender()
{
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

void RtpSender::addDerived(GstElement * newSrc, const MediaConfig & config)
{
    rtp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtp_sender_);
    g_object_set(rtp_sender_, "host", config.remoteHost(), "port", config.port(), "sync",
                 FALSE, "async", FALSE, NULL);

    rtcp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtcp_sender_);
    g_object_set(rtcp_sender_, "host", config.remoteHost(), "port", config.port() + 1, "sync",
                 FALSE, "async", FALSE, NULL);

    rtcp_receiver_ = gst_element_factory_make("udpsrc", NULL);
    assert(rtcp_receiver_);
    g_object_set(rtcp_receiver_, "port", config.port() + 5, NULL);

    pipeline_.add(rtp_sender_);
    pipeline_.add(rtcp_sender_);
    pipeline_.add(rtcp_receiver_);

    GstPad *send_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("send_rtp_sink_"));
    assert(send_rtp_sink);

    GstPad *send_rtp_src = gst_element_get_static_pad(rtpbin_,
                                                      padStr("send_rtp_src_"));
    assert(send_rtp_src);

    GstPad *send_rtcp_src = gst_element_get_request_pad(rtpbin_,
                                                        padStr("send_rtcp_src_"));
    assert(send_rtcp_src);

    GstPad *recv_rtcp_sink = gst_element_get_request_pad(rtpbin_,
                                                         padStr("recv_rtcp_sink_"));
    assert(recv_rtcp_sink);

    GstPad *payloadSrc = gst_element_get_static_pad(newSrc, "src");
    assert(payloadSrc);
    GstPad *rtpSenderSink = gst_element_get_static_pad(rtp_sender_, "sink");
    assert(rtpSenderSink);
    GstPad *rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink");
    assert(rtcpSenderSink);
    GstPad *rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src");
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

RtpReceiver::RtpReceiver() : rtp_receiver_(0)
{
}

RtpReceiver::~RtpReceiver()
{
    assert(pipeline_.stop());
    pipeline_.remove(rtp_receiver_);
}

void RtpReceiver::set_caps(const char *capsStr)
{
    GstCaps *caps;
    caps = gst_caps_from_string(capsStr);
    assert(caps);
    g_object_set(G_OBJECT(rtp_receiver_), "caps", caps, NULL);
    gst_caps_unref(caps);
}

void RtpReceiver::addDerived(GstElement * newSink, const MediaConfig & config)
{
    rtp_receiver_ = gst_element_factory_make("udpsrc", NULL);
    assert(rtp_receiver_);
    g_object_set(rtp_receiver_, "port", config.port(), NULL);

    rtcp_receiver_ = gst_element_factory_make("udpsrc", NULL);
    assert(rtcp_receiver_);
    g_object_set(rtcp_receiver_, "port", config.port() + 1, NULL);

    rtcp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtcp_sender_);
    g_object_set(rtcp_sender_, "host", config.remoteHost(), "port", config.port() + 5, "sync", FALSE,
                 "async", FALSE, NULL);

    pipeline_.add(rtp_receiver_);
    pipeline_.add(rtcp_receiver_);
    pipeline_.add(rtcp_sender_);

    GstPad *recv_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtp_sink_"));
    assert(recv_rtp_sink);
    GstPad *send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_"));
    assert(send_rtcp_src);
    GstPad *recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_"));
    assert(recv_rtcp_sink);

    GstPad *rtpReceiverSrc = gst_element_get_static_pad(rtp_receiver_, "src");
    assert(rtpReceiverSrc);
    GstPad *rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src");
    assert(rtcpReceiverSrc);
    GstPad *rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink");
    assert(rtcpSenderSink);

    assert(gst_pad_link(rtpReceiverSrc, recv_rtp_sink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(rtcpReceiverSrc, recv_rtcp_sink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(send_rtcp_src, rtcpSenderSink) == GST_PAD_LINK_OK);

    // when pad is created, it must be linked to new sink
    g_signal_connect(rtpbin_, "pad-added", G_CALLBACK(Pipeline::cb_new_src_pad), (void *) newSink);

    // release request pads (in reverse order)
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    //gst_element_release_request_pad(rtpbin_, recv_rtp_src);
    gst_element_release_request_pad(rtpbin_, recv_rtp_sink);

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtpReceiverSrc));
}
