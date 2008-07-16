
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

#include <gst/gst.h>
#include <cassert>
#include <iostream>
#include <list>

#include "logWriter.h"
#include "rtpReceiver.h"
#include "mediaConfig.h"

std::list<GstElement *> RtpReceiver::newSinks_;


RtpReceiver::RtpReceiver() : rtp_receiver_(0)
{
}

RtpReceiver::~RtpReceiver()
{
    assert(pipeline_.stop());
    pipeline_.remove(rtp_receiver_);
    // FIXME: assumes this destructor will be called in the right order, maybe should 
    // be replaced by observer
    newSinks_.pop_front();
}

void RtpReceiver::set_caps(const char *capsStr)
{
    GstCaps *caps;
    caps = gst_caps_from_string(capsStr);
    assert(caps);
    g_object_set(G_OBJECT(rtp_receiver_), "caps", caps, NULL);
    gst_caps_unref(caps);
}

void RtpReceiver::cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, void *data)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG("Pad is already linked")
            return;
    }
    if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
    {   
        LOG("Pad is not a source");
        return;
    }
    if (strncmp(gst_pad_get_name(srcPad), "recv_rtp_src", 12))
    {
        LOG("Wrong pad");
        return;
    }
    LOG("Dynamic pad created, linking new srcpad to existing sinkpad.");
    std::cout << "SrcElement: " << gst_element_get_name(srcElement) << std::endl;
    std::cout << "SrcPad: " << gst_pad_get_name(srcPad) << std::endl;

    std::list<GstElement *> * sinkElements = (std::list<GstElement *> *) data;
    std::cout << "You have " << sinkElements->size() << " sinks stored. " << std::endl;
    
    std::list<GstElement *>::iterator iter;
    for (iter = sinkElements->begin(); iter != sinkElements->end(); ++iter)
        std::cout << "SinkElement: " << gst_element_get_name(*iter) << std::endl;

    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(sinkElements->front(), "sink");

    iter = sinkElements->begin();
    while (strncmp(gst_caps_to_string(gst_pad_get_caps(sinkPad)), gst_caps_to_string(gst_pad_get_caps(srcPad)), 37) 
            && iter != sinkElements->end())
    {
        sinkPad = gst_element_get_static_pad(*iter, "sink");
        ++iter;
    }
    std::cout << "SINKPAD CAPS: " << gst_caps_to_string(gst_pad_get_caps(sinkPad)) << std::endl;
    std::cout << "SRCPAD CAPS: " << gst_caps_to_string(gst_pad_get_caps(srcPad)) << std::endl;

    assert(gst_pad_link(srcPad, sinkPad) == GST_PAD_LINK_OK);
    gst_object_unref(sinkPad);
}

void RtpReceiver::addDerived(GstElement * newSink, const MediaConfig * config)
{
    rtp_receiver_ = gst_element_factory_make("udpsrc", NULL);
    assert(rtp_receiver_);
    g_object_set(rtp_receiver_, "port", config->port(), NULL);

    rtcp_receiver_ = gst_element_factory_make("udpsrc", NULL);
    assert(rtcp_receiver_);
    g_object_set(rtcp_receiver_, "port", config->port() + 1, NULL);

    rtcp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtcp_sender_);
    g_object_set(rtcp_sender_, "host", config->remoteHost(), "port", config->port() + 5, "sync", FALSE,
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

    newSinks_.push_back(newSink);
    // when pad is created, it must be linked to new sink
    g_signal_connect(rtpbin_, "pad-added", G_CALLBACK(RtpReceiver::cb_new_src_pad), (void *) &newSinks_);

    // release request pads (in reverse order)
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    gst_element_release_request_pad(rtpbin_, recv_rtp_sink);

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtpReceiverSrc));
}

