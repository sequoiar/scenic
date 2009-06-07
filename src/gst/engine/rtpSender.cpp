/* rtpSender.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include <gst/gst.h>
#include <algorithm>
#include <sstream>

#include "pipeline.h"
#include "gstLinkable.h"
#include "rtpSender.h"
#include "rtpPay.h"
#include "remoteConfig.h"


void RtpSender::enableControl()
{
    Payloader::enableControl();
}

RtpSender::~RtpSender()
{
    Pipeline::Instance()->remove(&rtp_sender_);
}


void RtpSender::add(RtpPay * newSrc, const SenderConfig & config)
{
    RtpBin::init();
    registerSession(config.codec());

    GstPad *send_rtp_sink;
    GstPad *send_rtp_src;
    GstPad *send_rtcp_src;
    GstPad *recv_rtcp_sink;
    GstPad *payloadSrc;
    GstPad *rtpSenderSink;
    GstPad *rtcpSenderSink;
    GstPad *rtcpReceiverSrc;

    rtp_sender_ = Pipeline::Instance()->makeElement("udpsink", NULL);
    g_object_set(rtp_sender_, "host", config.remoteHost(), "port", config.port(), NULL);
    
    rtcp_sender_ = Pipeline::Instance()->makeElement("udpsink", NULL);
    g_object_set(rtcp_sender_, "host", config.remoteHost(), "port", config.rtcpFirstPort(),
                 "sync", FALSE, "async", FALSE, NULL);

    rtcp_receiver_ = Pipeline::Instance()->makeElement("udpsrc", NULL);
    g_object_set(rtcp_receiver_, "port", config.rtcpSecondPort(), NULL);
    

    // padStr adds a session id to the pad name, so we get the pad for this session
    send_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("send_rtp_sink_"));
    tassert(send_rtp_sink);
    send_rtp_src = gst_element_get_static_pad(rtpbin_, padStr("send_rtp_src_"));
    tassert(send_rtp_src);
    send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_"));
    tassert(send_rtcp_src);
    recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_"));
    tassert(recv_rtcp_sink);

    tassert(payloadSrc = gst_element_get_static_pad(newSrc->srcElement(), "src"));
    tassert(rtpSenderSink = gst_element_get_static_pad(rtp_sender_, "sink"));
    tassert(rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink"));
    tassert(rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src"));

    // link pads
    tassert(gstlinkable::link_pads(payloadSrc, send_rtp_sink));
    tassert(gstlinkable::link_pads(send_rtp_src, rtpSenderSink));
    tassert(gstlinkable::link_pads(send_rtcp_src, rtcpSenderSink));
    tassert(gstlinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink));
    
    // release request and static pads (in reverse order)
    gst_object_unref(GST_OBJECT(send_rtp_src)); // static pad
    // release request pads
    gst_element_release_request_pad(rtpbin_, send_rtp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtpSenderSink));
    gst_object_unref(GST_OBJECT(payloadSrc));

}


void RtpSender::checkSampleRate()
{
    GstPad *sinkPad = gst_element_get_pad(rtp_sender_, "sink");
    GstCaps *sinkCaps = gst_pad_get_negotiated_caps(sinkPad);
    //GstBase::checkCapsSampleRate(sinkCaps);

    gst_caps_unref(sinkCaps);
    gst_object_unref(sinkPad);
}


void RtpSender::subParseSourceStats(GstStructure *stats)
{
    const GValue *val = gst_structure_get_value(stats, "internal");
    if (g_value_get_boolean(val))   // is-internal
    {
        val = gst_structure_get_value(stats, "is-sender");
        if (g_value_get_boolean(val))    // is-sender
        {
            printStatsVal(sessionName_, "bitrate", "guint64", ":BITRATE: ", stats);
            printStatsVal(sessionName_, "octets-sent", "guint64", ":OCTETS-SENT:", stats);
            printStatsVal(sessionName_, "packets-sent", "guint64", ":PACKETS-SENT:", stats);
        }
        return; // otherwise we don't care about internal sources
    }
    printStatsVal(sessionName_, "rb-jitter", "guint32", ":JITTER: ", stats);
    printStatsVal(sessionName_, "rb-packetslost", "gint32", ":PACKETS LOST: ", stats);
}

