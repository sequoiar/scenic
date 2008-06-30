
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

#include "rtpSession.h"
#include "mediaConfig.h"
#include "mediaBase.h"
#include "logWriter.h"

// FIXME: Who should own pipeline? Should it have its own class?
bool RtpSession::init()
{
    rtpbin_ = gst_element_factory_make("gstrtpbin", NULL);
    assert(rtpbin_);
    
    pipeline_.add(rtpbin_);
}



const char *RtpSender::padStr(const char* padName)
{
    std::string result = padName + (rtp_senders_.size() - 1);
    return result.c_str();
}



const char *RtpReceiver::padStr(const char* padName)
{
    std::string result = padName + (rtp_receivers_.size() - 1);
    return result.c_str();
}



void RtpSender::addSource(GstElement *newSrc, MediaConfig &config)
{
    GstElement *rtp_sender, *rtcp_sender, *rtcp_receiver;

    rtp_sender = gst_element_factory_make("udpsink", NULL);
    assert(rtp_sender);
    g_object_set(rtp_sender, "host", config.remoteHost(), "port", config.port(), "sync", 
            FALSE, "async", FALSE, NULL);
    rtp_senders_.push_back(rtp_sender);
    
    rtcp_sender = gst_element_factory_make("udpsink", NULL);
    assert(rtcp_sender);
    g_object_set(rtcp_sender, "host", config.remoteHost(), "port", config.port() + 1, "sync", 
            FALSE, "async", FALSE, NULL);
    rtcp_senders_.push_back(rtcp_sender);
    
    rtcp_receiver = gst_element_factory_make("udpsrc", NULL);
    assert(rtcp_receiver);
    g_object_set(rtcp_receiver, "port", config.port() + 5, NULL);
    rtcp_receivers_.push_back(rtcp_receiver);

    pipeline_.add(rtpbin_);
    pipeline_.add(rtp_sender);
    pipeline_.add(rtcp_sender);
    pipeline_.add(rtcp_receiver);

    int sourceCount = rtp_senders_.size() - 1;
    assert(sourceCount >= 0);

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
    GstPad *rtpSenderSink = gst_element_get_static_pad(rtp_sender, "sink");
    assert(rtpSenderSink);
    GstPad *rtcpSenderSink = gst_element_get_static_pad(rtcp_sender, "sink");
    assert(rtcpSenderSink);
    GstPad *rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver, "src");
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



void RtpReceiver::addSink(GstElement *newSink, MediaConfig &config)
{
    GstElement *rtp_receiver = gst_element_factory_make("udpsrc", NULL);
    assert(rtp_receiver);
    g_object_set(rtp_receiver, "port", config.port(), NULL);
    rtp_receivers_.push_back(rtp_receiver);

    GstElement *rtcp_receiver = gst_element_factory_make("udpsrc", NULL);
    assert(rtcp_receiver);
    g_object_set(rtcp_receiver, "port", config.port() + 1, NULL);
    rtcp_receivers_.push_back(rtcp_receiver);

    GstElement *rtcp_sender = gst_element_factory_make("udpsink", NULL);
    assert(rtcp_sender);
	g_object_set(rtcp_sender, "host", config.remoteHost(), "port", config.port() + 5, "sync", FALSE, "async", FALSE, NULL);
    rtcp_senders_.push_back(rtcp_sender);

    pipeline_.add(rtpbin_);
    pipeline_.add(rtp_receiver);
    pipeline_.add(rtcp_receiver);
    pipeline_.add(rtcp_sender);

	GstPad *recv_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtp_sink_"));
    assert(recv_rtp_sink);
	GstPad *send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_"));
    assert(send_rtcp_src);
	GstPad *recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_"));
    assert(recv_rtcp_sink);

    GstPad *rtpReceiverSrc = gst_element_get_static_pad(rtp_receiver, "src");
    assert(rtpReceiverSrc);
    GstPad *rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver, "src");
    assert(rtcpReceiverSrc);
    GstPad *rtcpSenderSink = gst_element_get_static_pad(rtcp_sender, "sink");
    assert(rtcpSenderSink);

    assert(gst_pad_link(rtpReceiverSrc, recv_rtp_sink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(rtcpReceiverSrc, recv_rtcp_sink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(send_rtcp_src, rtcpSenderSink) == GST_PAD_LINK_OK);
	
    // when pad is created, it must be linked to new sink
    g_signal_connect(rtpbin_, "pad-added", G_CALLBACK(Pipeline::cb_new_src_pad), (void *)newSink);

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

