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


#ifdef CONFIG_DEBUG_LOCAL
#include <gtk/gtk.h>
GtkWidget *RtpReceiver::control_ = 0;
bool RtpReceiver::madeControl_ = false;
#endif


std::list<GstElement *> RtpReceiver::depayloaders_;


RtpReceiver::~RtpReceiver()
{
    Pipeline::Instance()->remove(&rtp_receiver_);

    // find this->depayloader in the static list of depayloaders
    std::list<GstElement *>::iterator iter;
    iter = std::find(depayloaders_.begin(), depayloaders_.end(), depayloader_);

    // make sure we found it and remove it
    assert(iter != depayloaders_.end());
    depayloaders_.erase(iter);

#ifdef CONFIG_DEBUG_LOCAL
    if (control_)
    {
        madeControl_ = false;
        gtk_widget_destroy(control_);
        LOG_DEBUG("RTP jitterbuffer control window destroyed");
        control_ = 0;
    }
#endif
}


void RtpReceiver::setCaps(const char *capsStr)
{
    GstCaps *caps;
    assert(caps = gst_caps_from_string(capsStr));
    g_object_set(G_OBJECT(rtp_receiver_), "caps", caps, NULL);
    gst_caps_unref(caps);
}


void RtpReceiver::checkSampleRate()
{
    GstPad *srcPad = gst_element_get_pad(rtp_receiver_, "src");
    GstCaps *srcCaps = gst_pad_get_negotiated_caps(srcPad);
    //GstBase::checkCapsSampleRate(srcCaps);
    gst_caps_unref(srcCaps);
    gst_object_unref(srcPad);
}


void RtpReceiver::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, void * /* data*/)
{
    // FIXME: Once this callback is attached to the pad-added signal, it gets called like crazy, any time any pad
    // is added (regardless of whether or not it's a dynamic pad) to rtpbin.
    //GstElement *depayloader = static_cast<GstElement*>(data);

    if (gst_pad_is_linked(srcPad))
    {
        LOG_DEBUG("Pad is already linked");
        return;
    }
    else if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
    {
        LOG_DEBUG("Pad is not a source");
        return;
    }
    else if (strncmp(gst_pad_get_name(srcPad), "recv_rtp_src", 12))
    {
        LOG_DEBUG("Wrong pad");
        return;
    }
    // FIXME: We only have this really moderately stupid method of comparing the caps of all
    // the sinks that have been attached to RtpReceiver's in general (stored in a static list) against those of the new pad.
    GstPad *sinkPad = getMatchingDepayloaderSinkPad(srcPad);

    if (gst_pad_is_linked(sinkPad)) // only link once
    {
        LOG_WARNING("sink pad is already linked.");
        gst_object_unref(sinkPad);
        return;
    }

    assert(gstlinkable::link_pads(srcPad, sinkPad));    // link our udpsrc to the corresponding depayloader

    gst_object_unref(sinkPad);
}


std::string RtpReceiver::getMediaType(GstPad *pad)
{
    GstStructure *structure = gst_caps_get_structure(gst_pad_get_caps(pad), 0);
    const GValue *str = gst_structure_get_value(structure, "media");
    std::string result(g_value_get_string(str));

    if (result != "video" && result != "audio")
        THROW_ERROR("Media type of depayloader sink pad is neither audio nor video!");

    return result;
}


GstPad *RtpReceiver::getMatchingDepayloaderSinkPad(GstPad *srcPad)
{
    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(depayloaders_.front(), "sink");

    // match depayloader to rtp pad by media type

    std::list<GstElement *>::iterator iter = depayloaders_.begin();
    std::string srcMediaType(getMediaType(srcPad));

    while (getMediaType(sinkPad) != srcMediaType
            && iter != depayloaders_.end())
    {
        gst_object_unref(sinkPad);
        sinkPad = gst_element_get_static_pad(*iter, "sink");
        ++iter;
    }

    return sinkPad;
}


void RtpReceiver::add(RtpPay * depayloader, const ReceiverConfig & config)
{
    RtpBin::init();

    GstPad *recv_rtp_sink;
    GstPad *send_rtcp_src;
    GstPad *recv_rtcp_sink;
    GstPad *rtpReceiverSrc;
    GstPad *rtcpReceiverSrc;
    GstPad *rtcpSenderSink;

    // store copy so that destructor knows which depayloader to remove from its list
    depayloader_ = depayloader->sinkElement();

    rtp_receiver_ = Pipeline::Instance()->makeElement("udpsrc", NULL);
    g_object_set(rtp_receiver_, "port", config.port(), NULL);

    rtcp_receiver_ = Pipeline::Instance()->makeElement("udpsrc", NULL);
    g_object_set(rtcp_receiver_, "port", config.port() + 1, NULL);

    rtcp_sender_ = Pipeline::Instance()->makeElement("udpsink", NULL);
    g_object_set(rtcp_sender_, "host", config.remoteHost(), "port",
            config.port() + 5, "sync", FALSE, "async", FALSE, NULL);

    assert(recv_rtp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtp_sink_")));
    assert(send_rtcp_src = gst_element_get_request_pad(rtpbin_, padStr("send_rtcp_src_")));
    assert(recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, padStr("recv_rtcp_sink_")));

    assert(rtpReceiverSrc = gst_element_get_static_pad(rtp_receiver_, "src"));
    assert(rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src"));
    assert(rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink"));

    assert(gstlinkable::link_pads(rtpReceiverSrc, recv_rtp_sink));
    assert(gstlinkable::link_pads(rtcpReceiverSrc, recv_rtcp_sink));
    assert(gstlinkable::link_pads(send_rtcp_src, rtcpSenderSink));

    depayloaders_.push_back(depayloader_);
    // when pad is created, it must be linked to new sink
    g_signal_connect(rtpbin_, "pad-added", 
            G_CALLBACK(RtpReceiver::cb_new_src_pad), 
            NULL);

    // release request pads (in reverse order)
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    gst_element_release_request_pad(rtpbin_, recv_rtp_sink);

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtpReceiverSrc));

#ifdef CONFIG_DEBUG_LOCAL
    createLatencyControl();
#endif
}


void RtpReceiver::updateLatencyCb(GtkAdjustment *adj)
{
    unsigned val = static_cast<unsigned>(adj->value);
    g_print("Setting latency to %d\n", val);
    RtpBin::setLatency(val);
}

/* makes the latency window */
void RtpReceiver::createLatencyControl()
{
    if (madeControl_)
        return;

    static bool gtk_initialized = false;
    if (!gtk_initialized)
    {
        gtk_init(0, NULL);
        gtk_initialized = true;
    }

    GtkWidget *box1;
    GtkWidget *hscale;
    GtkObject *adj;
    const int WIDTH = 400;
    const int HEIGHT = 70;

    /* Standard window-creating stuff */
    control_ = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(control_), WIDTH, HEIGHT);
    gtk_window_set_title (GTK_WINDOW (control_), "Rtpjitterbuffer Latency (ms)");

    box1 = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (control_), box1);
    gtk_widget_show (box1);

    /* value, lower, upper, step_increment, page_increment, page_size */
    /* Note that the page_size value only makes a difference for
     * scrollbar widgets, and the highest value you'll get is actually
     * (upper - page_size). */
    adj = gtk_adjustment_new (3.0, 1.0, 201.0, 1.0, 1.0, 1.0);

    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
            GTK_SIGNAL_FUNC(updateLatencyCb), NULL);


    hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    // Signal emitted only when value is done changing
    gtk_range_set_update_policy (GTK_RANGE (hscale), 
            GTK_UPDATE_DISCONTINUOUS);
    gtk_box_pack_start (GTK_BOX (box1), hscale, TRUE, TRUE, 0);
    gtk_widget_show (hscale);

    gtk_widget_show (control_);
    madeControl_ = true;
}

