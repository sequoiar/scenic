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


void RtpSender::sendCapsChanged(GstPad *pad, GParamSpec * /*pspec*/, RtpSender* context)
{
    GstCaps *caps = NULL;

    g_object_get(pad, "caps", &caps, NULL);

    if (!caps)
        return;
    
    g_return_if_fail (GST_CAPS_IS_SIMPLE(caps));

    // post msg with caps on bus, where some worker (thread? async callback?) will send it to the receiver
    gst_element_post_message(context->rtp_sender_, 
            gst_message_new_application(GST_OBJECT(context->rtp_sender_), gst_structure_new("caps-changed", "caps", G_TYPE_STRING, gst_caps_to_string(caps), NULL)));

    gst_caps_unref(caps);
}


/** 
 * The new caps message is posted on the bus by the src pad of our udpsink, 
 * received by this rtpsender, and dispatched. */
// FIXME: maybe someone else should deal with this? or maybe it should be in a separate thread?
bool RtpSender::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);

    if (std::string(name) == "caps-changed") 
    {   
        // this is our msg
        const gchar *newCapsStr = gst_structure_get_string(s, "caps");
        assert(newCapsStr);
        gchar *host;
        g_object_get(G_OBJECT(rtp_sender_), "host", &host, NULL);
        // FIXME: have this codec list stored somewhere else
        if (config_->codec() == "theora" 
                or config_->codec() == "vorbis" 
                or config_->codec() == "mp3" 
                or config_->codec() == "raw")
            config_->sendMessage(std::string(newCapsStr));

        g_free(host);
        
        return true;
    }

    return false;           // this wasn't our msg, someone else should handle it
}


void RtpSender::add(RtpPay * newSrc, const SenderConfig & config)
{
    RtpBin::init();
    config_ = std::tr1::shared_ptr<SenderConfig>(new SenderConfig(config));
    registerSession(config_->codec());
    // register this rtpSender to handle new caps msg
    Pipeline::Instance()->subscribe(this);

    GstPad *send_rtp_sink;
    GstPad *send_rtp_src;
    GstPad *send_rtcp_src;
    GstPad *recv_rtcp_sink;
    GstPad *payloadSrc;
    GstPad *rtpSenderSink;
    GstPad *rtcpSenderSink;
    GstPad *rtcpReceiverSrc;

    rtp_sender_ = Pipeline::Instance()->makeElement("udpsink", NULL);
    g_object_set(rtp_sender_, "host", config_->remoteHost(), "port", config_->port(), NULL);
    
    rtcp_sender_ = Pipeline::Instance()->makeElement("udpsink", NULL);
    g_object_set(rtcp_sender_, "host", config_->remoteHost(), "port", config_->rtcpFirstPort(),
                 "sync", FALSE, "async", FALSE, NULL);

    rtcp_receiver_ = Pipeline::Instance()->makeElement("udpsrc", NULL);
    g_object_set(rtcp_receiver_, "port", config_->rtcpSecondPort(), NULL);
    
    // padStr adds a session id to the pad name, so we get the pad for this session
    /* now link all to the rtpbin, start by getting an RTP sinkpad for session n */
    send_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("send_rtp_sink_"));
    tassert(send_rtp_sink);

    g_signal_connect(send_rtp_sink, "notify::caps", G_CALLBACK(sendCapsChanged), this);

    tassert(payloadSrc = gst_element_get_static_pad(newSrc->srcElement(), "src"));
    tassert(gstlinkable::link_pads(payloadSrc, send_rtp_sink));
    gst_object_unref(GST_OBJECT(payloadSrc));

    send_rtp_src = gst_element_get_static_pad(rtpbin_, padStr("send_rtp_src_"));
    tassert(send_rtp_src);
    tassert(rtpSenderSink = gst_element_get_static_pad(rtp_sender_, "sink"));
    tassert(gstlinkable::link_pads(send_rtp_src, rtpSenderSink));
    gst_object_unref(send_rtp_src); // static pad


    /* get an RTCP srcpad for sending RTCP to the receiver */
    tassert(send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_")));
    tassert(rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink"));
    tassert(gstlinkable::link_pads(send_rtcp_src, rtcpSenderSink));
    gst_object_unref(rtcpSenderSink);

    /* we also want to receive RTCP, request an RTCP sinkpad for session n and
     * link it to the srcpad of the udpsrc for RTCP */
    tassert(rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src"));
    tassert(recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_")));
    tassert(gstlinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink));
    gst_object_unref(rtcpReceiverSrc);
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

