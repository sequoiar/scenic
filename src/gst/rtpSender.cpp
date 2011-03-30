/* rtpSender.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gst/gst.h>
#include <algorithm>
#include <sstream>
#include <list>

#include "pipeline.h"
#include "gstLinkable.h"
#include "rtpSender.h"
#include "rtpPay.h"
#include "remoteConfig.h"


RtpSender::~RtpSender()
{
    /// remove request pads
    if (send_rtp_sink_)
        gst_object_unref(send_rtp_sink_);
    if (send_rtcp_src_)
        gst_object_unref(send_rtcp_src_);
    if (recv_rtcp_sink_)
        gst_object_unref(recv_rtcp_sink_);
}


void RtpSender::sendCapsChanged(GstPad *pad, GParamSpec * /*pspec*/, RtpSender* context)
{
    GstCaps *caps = NULL;

    g_object_get(pad, "caps", &caps, NULL);

    if (!caps)
        return;

    g_return_if_fail (GST_CAPS_IS_SIMPLE(caps));

    // post msg with caps on bus, where some worker (thread? async callback?) will send it to the receiver
    gst_element_post_message(context->rtp_sender_,
            gst_message_new_application(GST_OBJECT(context->rtp_sender_),
                gst_structure_new("caps-changed", "caps", G_TYPE_STRING,
                    gst_caps_to_string(caps), NULL)));

    gst_caps_unref(caps);
}


void RtpSender::onNewSSRC(GstElement * /*rtpbin*/, guint session, guint ssrc, gpointer /*data*/)
{
    /// FIXME: make sure this is correct
    LOG_INFO("New ssrc " << ssrc << " for session " << sessions_[session]->sessionName());
}


void RtpSender::add(RtpPay * newSrc, const SenderConfig & config)
{
    registerSession(config.identifier());

    GstPad *send_rtp_src;
    GstPad *payloadSrc;
    GstPad *rtpSenderSink;
    GstPad *rtcpSenderSink;
    GstPad *rtcpReceiverSrc;

    /// FIXME: need to update config.ports() accordingly if they can change (which for now they can't)
    rtp_sender_ = pipeline_.makeElement("udpsink", NULL);
    int rtpsink_socket = RtpBin::createSinkSocket(config.remoteHost(), config.port());
    g_object_set(rtp_sender_, "sockfd", rtpsink_socket, "host",
            config.remoteHost(), "port", config.port(), NULL);

    rtcp_sender_ = pipeline_.makeElement("udpsink", NULL);
    int rtcpsink_socket = RtpBin::createSinkSocket(config.remoteHost(), config.rtcpFirstPort());
    g_object_set(rtcp_sender_, "sockfd", rtcpsink_socket, "host", config.remoteHost(),
            "port", config.rtcpFirstPort(), "sync", FALSE, "async", FALSE, NULL);

    rtcp_receiver_ = pipeline_.makeElement("udpsrc", NULL);
    int rtcpsrc_socket = RtpBin::createSourceSocket(config.rtcpSecondPort());
    g_object_set(rtcp_receiver_, "sockfd", rtcpsrc_socket, "port", config.rtcpSecondPort(), NULL);

    // padStr adds a session id to the pad name, so we get the pad for this session
    /* now link all to the rtpbin, start by getting an RTP sinkpad for session n */
    send_rtp_sink_ = gst_element_get_request_pad(rtpbin_, padStr("send_rtp_sink_"));
    assert(send_rtp_sink_);

    g_signal_connect(send_rtp_sink_, "notify::caps", G_CALLBACK(sendCapsChanged), this);
    g_signal_connect(rtpbin_, "on-new-ssrc", G_CALLBACK(onNewSSRC), this);

    payloadSrc = gst_element_get_static_pad(newSrc->srcElement(), "src");
    assert(payloadSrc);
    bool linked = gstlinkable::link_pads(payloadSrc, send_rtp_sink_);
    assert(linked);
    gst_object_unref(GST_OBJECT(payloadSrc));

    send_rtp_src = gst_element_get_static_pad(rtpbin_, padStr("send_rtp_src_"));
    assert(send_rtp_src);
    rtpSenderSink = gst_element_get_static_pad(rtp_sender_, "sink");
    assert(rtpSenderSink);
    linked = gstlinkable::link_pads(send_rtp_src, rtpSenderSink);
    assert(linked);
    gst_object_unref(send_rtp_src); // static pad


    /* get an RTCP srcpad for sending RTCP to the receiver */
    send_rtcp_src_ = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_"));
    assert(send_rtcp_src_);
    rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink");
    assert(rtcpSenderSink);
    linked = gstlinkable::link_pads(send_rtcp_src_, rtcpSenderSink);
    assert(linked);
    gst_object_unref(rtcpSenderSink);

    /* we also want to receive RTCP, request an RTCP sinkpad for session n and
     * link it to the srcpad of the udpsrc for RTCP */
    rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src");
    assert(rtcpReceiverSrc);
    recv_rtcp_sink_ = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_"));
    assert(recv_rtcp_sink_);
    linked = gstlinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink_);
    assert(linked);
    gst_object_unref(rtcpReceiverSrc);
}


void RtpSender::deltaPacketLoss(GstStructure *stats)
{
    using std::string;
    using std::map;
    using std::list;
    static map<string, list<gint32> > packetLoss;
    static map<string, list<gint32> > packetsSent;

    if (G_VALUE_HOLDS_INT(gst_structure_get_value(stats, "rb-packetslost")))
        packetLoss[sessionName_].push_back(g_value_get_int(gst_structure_get_value(stats, "rb-packetslost")));
    if (G_VALUE_HOLDS_UINT64(gst_structure_get_value(stats, "packets-sent")))
    {
        packetsSent[sessionName_].push_back(g_value_get_uint64(gst_structure_get_value(stats, "packets-sent")));
        return;
    }

    const size_t WINDOW_SIZE = 10;

    // get difference between newest and oldest packetloss values
    const double deltaLoss = packetLoss[sessionName_].back() - packetLoss[sessionName_].front();
    const double deltaSent = packetsSent[sessionName_].back() - packetsSent[sessionName_].front();
    // our old data is no longer valid, need to reset
    if (deltaLoss < 0.0 or deltaSent < 0.0)
    {
        packetLoss[sessionName_].resize(0);
        packetsSent[sessionName_].resize(0);
    }
    else if (deltaSent > 0.0)
        LOG_INFO(sessionName_ << ":AVERAGE PACKET-LOSS(%):" << 100.0 * (deltaLoss / deltaSent));

    while (packetLoss[sessionName_].size() > WINDOW_SIZE) // while buffer is overfull
        packetLoss[sessionName_].pop_front();  // pop oldest element
    while (packetsSent[sessionName_].size() > WINDOW_SIZE) // while buffer is overfull
        packetsSent[sessionName_].pop_front();  // pop oldest element
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
            deltaPacketLoss(stats);
        }
        return; // otherwise we don't care about internal sources
    }
    printStatsVal(sessionName_, "rb-jitter", "guint32", ":JITTER: ", stats);
    printStatsVal(sessionName_, "rb-packetslost", "gint32", ":PACKETS-LOST: ", stats);
    deltaPacketLoss(stats);
}

