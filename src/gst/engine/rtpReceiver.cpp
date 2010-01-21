/* rtpReceiver.cpp
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

#include <list>
#include <algorithm>
#include <cstring>
#include <gst/gst.h>

#include "gstLinkable.h"
#include "pipeline.h"
#include "rtpPay.h"
#include "rtpReceiver.h"
#include "remoteConfig.h"
#include "playback.h"

#include <gtk/gtk.h>

bool RtpReceiver::controlEnabled_ = false;
bool RtpReceiver::madeControl_ = false;
GtkWidget *RtpReceiver::control_ = 0;


std::list<GstElement *> RtpReceiver::depayloaders_;

void RtpReceiver::enableControl() 
{ 
    controlEnabled_ = true; 
}

RtpReceiver::~RtpReceiver()
{
    Pipeline::Instance()->remove(&rtp_receiver_);

    // find this->depayloader in the static list of depayloaders
    if (depayloader_) // in case destructor was called before depayloader was created
    {
        std::list<GstElement *>::iterator iter;
        iter = std::find(depayloaders_.begin(), depayloaders_.end(), depayloader_);

        // make sure we found it and remove it
        tassert(iter != depayloaders_.end());
        depayloaders_.erase(iter);
    }

    if (control_)
    {
        madeControl_ = false;
        gtk_widget_destroy(control_);
        LOG_DEBUG("RTP jitterbuffer control window destroyed");
        control_ = 0;
    }
}


void RtpReceiver::setLatency(int latency)
{
    tassert(rtpbin_);
    if (latency < MIN_LATENCY or latency > MAX_LATENCY)
        THROW_ERROR("Cannot set rtpbin latency to " << latency << ", must be in range "
                << MIN_LATENCY << " to " << MAX_LATENCY);
    g_object_set(G_OBJECT(rtpbin_), "latency", latency, NULL);
}



void RtpReceiver::setCaps(const char *capsStr)
{
    GstCaps *caps;
    if (std::string("") == capsStr)
        THROW_ERROR("Cannot set rtp receiver caps to empty string");
    tassert(caps = gst_caps_from_string(capsStr));
    g_object_set(G_OBJECT(rtp_receiver_), "caps", caps, NULL);

    gst_caps_unref(caps);
}


void RtpReceiver::onPadAdded(GstElement *  /*rtpbin*/, GstPad * srcPad, void * /* data*/)
{
    // don't look at the full name
    static const std::string expectedPadPrefix = "recv_rtp_src";
    if (gst_pad_is_linked(srcPad))
        LOG_DEBUG("Pad is already linked");
    else if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
        LOG_DEBUG("Pad is not a source");
    else if (std::string(gst_pad_get_name(srcPad)).compare(0, expectedPadPrefix.length(), expectedPadPrefix))
        LOG_DEBUG("Wrong pad");
    else
    {
        GstPad *sinkPad = getMatchingDepayloaderSinkPad(srcPad);

        if (gst_pad_is_linked(sinkPad)) // only link once
        {
            LOG_WARNING("sink pad is already linked, are you trying to connect a new sender to an "
                    "existing receiver that lost its sender? You may have specified the wrong port numbers."
                    "This will likely crash.");
            gst_object_unref(sinkPad);
            return;
        }

        gstlinkable::link_pads(srcPad, sinkPad);    // link our udpsrc to the corresponding depayloader
        gchar *srcPadName;
        srcPadName = gst_pad_get_name(srcPad);
        /// FIXME: name by itself isn't so helpful, also sinkPad name is just sink so we ignore it
        LOG_INFO("Made new RTP connection with source pad " << srcPadName);
        g_free(srcPadName);

        gst_object_unref(sinkPad);
    }
}


void RtpReceiver::onSenderTimeout(GstElement *  /*rtpbin*/, guint /* session */, guint /* ssrc */, gpointer data)
{
    LOG_INFO("Sender timeout, quitting.");
    RtpReceiver *context = static_cast<RtpReceiver*>(data);
    context->printStats_ = false;
    //playback::quit(); // can't do this here without crashing
}


std::string RtpReceiver::getMediaType(GstPad *pad)
{
    GstStructure *structure = gst_caps_get_structure(gst_pad_get_caps(pad), 0);
    const GValue *str = gst_structure_get_value(structure, "media");
    std::string result(g_value_get_string(str));

    if (result != "video" and result != "audio")
        THROW_ERROR("Media type of depayloader sink pad is neither audio nor video!");

    return result;
}


GstPad *RtpReceiver::getMatchingDepayloaderSinkPad(GstPad *srcPad)
{
    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(depayloaders_.front(), "sink");

    // match depayloader to rtp pad by media type
    // FIXME: what if we have two video depayloaders? two audio depayloaders?

    std::list<GstElement *>::iterator iter = depayloaders_.begin();
    std::string srcMediaType(getMediaType(srcPad));

    while (getMediaType(sinkPad) != srcMediaType
            and iter != depayloaders_.end())
    {
        gst_object_unref(sinkPad);
        sinkPad = gst_element_get_static_pad(*iter, "sink");
        ++iter;
    }

    return sinkPad;
}


void RtpReceiver::add(RtpPay * depayloader, const ReceiverConfig & config)
{
    registerSession(config.codec());

    // KEEP THIS LOW OR SUFFER THE CONSEQUENCES
    // rule of thumb: 2-3 times the maximum network jitter
    setLatency(INIT_LATENCY);

    GstPad *recv_rtp_sink;
    GstPad *send_rtcp_src;
    GstPad *recv_rtcp_sink;
    GstPad *rtpReceiverSrc;
    GstPad *rtcpReceiverSrc;
    GstPad *rtcpSenderSink;

    // store copy so that destructor knows which depayloader to remove from its list
    depayloader_ = depayloader->sinkElement();
    // add to our list of active depayloaders
    depayloaders_.push_back(depayloader_);

    rtp_receiver_ = Pipeline::Instance()->makeElement("udpsrc", NULL);
    int rtpsrc_socket = RtpBin::createSourceSocket(config.port());
    g_object_set(rtp_receiver_, "sockfd", rtpsrc_socket, "port", config.port(), NULL);

    // this is a multicast session
    if (config.hasMulticastInterface())
    {
        g_object_set(rtp_receiver_, "multicast-group", config.remoteHost(), 
                "multicast-iface", config.multicastInterface(), NULL);
        LOG_DEBUG("Using IFACE for multicast" << config.multicastInterface());
    }

    rtcp_receiver_ = Pipeline::Instance()->makeElement("udpsrc", NULL);
    int rtcpsrc_socket = RtpBin::createSourceSocket(config.rtcpFirstPort());
    g_object_set(rtcp_receiver_, "sockfd", rtcpsrc_socket, "port", config.rtcpFirstPort(), NULL);

    rtcp_sender_ = Pipeline::Instance()->makeElement("udpsink", NULL);
    int rtcpsink_socket = RtpBin::createSinkSocket(config.remoteHost(), config.rtcpSecondPort());
    g_object_set(rtcp_sender_, "host", config.remoteHost(), "sockfd", rtcpsink_socket, "port",
            config.rtcpSecondPort(), "sync", FALSE, "async", FALSE, NULL);

    /* now link all to the rtpbin, start by getting an RTP sinkpad for session n */
    tassert(rtpReceiverSrc = gst_element_get_static_pad(rtp_receiver_, "src"));
    tassert(recv_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtp_sink_")));
    tassert(gstlinkable::link_pads(rtpReceiverSrc, recv_rtp_sink));
    gst_object_unref(rtpReceiverSrc);

    /* get an RTCP sinkpad in session n */
    tassert(rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src"));
    tassert(recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_")));
    tassert(gstlinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink));
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));

    /* get an RTCP srcpad for sending RTCP back to the sender */
    tassert(send_rtcp_src = gst_element_get_request_pad (rtpbin_, padStr("send_rtcp_src_")));
    tassert(rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink"));
    tassert(gstlinkable::link_pads(send_rtcp_src, rtcpSenderSink));
    gst_object_unref(rtcpSenderSink);

    // when pad is created, it must be linked to new sink
    g_signal_connect(rtpbin_, "pad-added", 
            G_CALLBACK(RtpReceiver::onPadAdded), 
            NULL);

    g_signal_connect(rtpbin_, "on-sender-timeout", 
            G_CALLBACK(RtpReceiver::onSenderTimeout), 
            this);

    if (controlEnabled_)
        createLatencyControl();
}


void RtpReceiver::updateLatencyCb(GtkWidget *scale)
{
    unsigned val = static_cast<unsigned>(gtk_range_get_value(GTK_RANGE(scale)));
    LOG_DEBUG("Setting latency to " << val);
    setLatency(val);
}

/* makes the latency window */
void RtpReceiver::createLatencyControl()
{
    if (madeControl_)   // one control sets all jitterbuffers
        return;

    static bool gtk_initialized = false;
    if (!gtk_initialized)
    {
        gtk_init(0, NULL);
        gtk_initialized = true;
    }

    GtkWidget *box1;
    GtkWidget *hscale;
    const int WIDTH = 400;
    const int HEIGHT = 70;

    /* Standard window-creating stuff */
    control_ = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(control_), WIDTH, HEIGHT);
    gtk_window_set_title (GTK_WINDOW (control_), "Rtpjitterbuffer Latency (ms)");

    box1 = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (control_), box1);

    /* value, lower, upper, step_increment, page_increment, page_size */
    /* Note that the page_size value only makes a difference for
     * scrollbar widgets, and the highest value you'll get is actually
     * (upper - page_size). */

    hscale = gtk_hscale_new_with_range(MIN_LATENCY, MAX_LATENCY, 1.0);
    gtk_range_set_value(GTK_RANGE(hscale), INIT_LATENCY);
    gtk_signal_connect (GTK_OBJECT(hscale), "value_changed",
            GTK_SIGNAL_FUNC(updateLatencyCb), NULL);

    // Signal emitted only when value is done changing
    gtk_range_set_update_policy (GTK_RANGE (hscale), 
            GTK_UPDATE_DISCONTINUOUS);
    gtk_box_pack_start (GTK_BOX (box1), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);
    gtk_widget_show (box1);
    gtk_widget_show (control_);
    madeControl_ = true;
}


void RtpReceiver::subParseSourceStats(GstStructure *stats)
{
    const GValue *val = gst_structure_get_value(stats, "internal");
    if (g_value_get_boolean(val))   // is-internal
        return;

    printStatsVal(sessionName_, "octets-received", "guint64", ":OCTETS-RECEIVED:", stats);
    printStatsVal(sessionName_, "packets-received", "guint64", ":PACKETS-RECEIVED:", stats);
}

