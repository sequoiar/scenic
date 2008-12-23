/* rtpBin.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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
#include <cstring>
#include "rtpBin.h"
#include "rtpPay.h"
#include "remoteConfig.h"
#include "pipeline.h"


GstElement *RtpBin::rtpbin_ = 0;
GObject *RtpBin::session_ = 0;
unsigned int RtpBin::refCount_ = 0;

void RtpBin::init()
{
    // only initialize rtpbin once per process
    if (rtpbin_ == 0) 
        rtpbin_ = Pipeline::Instance()->makeElement("gstrtpbin", NULL);
    
    g_object_set(G_OBJECT(rtpbin_), "latency", 1, NULL);
    
#if 0
    // uncomment this to see each gstrtpsession's bandwidth printed
    // connect our action signal handler which requests our internal session, to our got-session handler.
    g_signal_connect(G_OBJECT(rtpbin_), "get-internal-session", G_CALLBACK(gotInternalSessionCb), NULL);
    g_timeout_add(5000 /* ms */, 
                  static_cast<GSourceFunc>(printBandwidth),
                  static_cast<void*>(this));
#endif
}


/// set drop-on-latency to TRUE, needs to be called upon creation of jitterbuffers, via a signal handler.
/// No visible effect. 

int RtpBin::dropOnLatency(gpointer data)
{
    RtpBin *context = static_cast<RtpBin*>(data);
    for (unsigned int sessionId = 0; sessionId < context->refCount_; ++sessionId)
        context->dropOnLatency(sessionId);

    return FALSE; // called once
}


void RtpBin::dropOnLatency(guint sessionID)
{
    GValue val;
    memset(&val, 0, sizeof(val));
    g_value_init(&val, G_TYPE_BOOLEAN);
    g_value_set_boolean(&val, TRUE);

    std::stringstream jitterbufferName;
    jitterbufferName << "rtpjitterbuffer" << sessionID << "::drop-on-latency";
    gst_child_proxy_set_property(GST_OBJECT(rtpbin_), jitterbufferName.str().c_str(), &val);
}


/// print jitter for each source
int RtpBin::printJitter(gpointer data)
{
    RtpBin *context = static_cast<RtpBin*>(data);
    for (unsigned int sessionId = 0; sessionId < context->refCount_; ++sessionId)
        LOG_INFO("Jitter: " << context->jitter(sessionId));

    return FALSE; // called once
}


/// Arbitrarily increase bandwidth, this function exists just to show how one can
/// set the bandwidth on each session. Currently seems to have no effect.

int RtpBin::increaseBandwidth(gpointer data)
{
    RtpBin *context = static_cast<RtpBin*>(data);
    const static gdouble newBandwidth = 100000.0;

    for (unsigned int sessionId = 0; sessionId < context->refCount_; ++sessionId)
    {
        context->bandwidth(sessionId, newBandwidth);
        LOG_INFO("BANDWIDTH USED: " << context->bandwidth(sessionId));
    }

    return FALSE;       // only called once
}


int RtpBin::printBandwidth(gpointer data)
{
    RtpBin *context = static_cast<RtpBin*>(data);
    std::string terminator("");

    for (unsigned int sessionId = 0; sessionId < context->refCount_; ++sessionId)
    {
        LOG_INFO(context->bandwidth(sessionId) << " bits/s" << terminator);
        terminator = "\n\n";
    }

    return TRUE;
}


const char *RtpBin::padStr(const char *padName)
{
    std::string result(padName);
    std::stringstream istream;

    istream << refCount_ - 1;        // 0-based
    result = result + istream.str();
    return result.c_str();
}


RtpBin::~RtpBin()
{
    Pipeline::Instance()->remove(&rtcp_sender_);
    Pipeline::Instance()->remove(&rtcp_receiver_);

    --refCount_;
    if (refCount_ <= 0) // destroy if no streams are present
    {
        assert(refCount_ == 0);
        Pipeline::Instance()->remove(&rtpbin_);
        rtpbin_ = 0;
    }
}


/// Sets bandwidth for the RtpSession specified by sessionID.

void RtpBin::bandwidth(guint sessionId, double newBandwidth) 
{
    requestSession(sessionId);
    g_object_set(G_OBJECT(session_), "bandwidth", newBandwidth, NULL);
}


/// Gets bandwidth for the RtpSession specified by sessionID.

double RtpBin::bandwidth(guint sessionId) const
{
    gdouble result = 0.0;

    requestSession(sessionId);
    g_object_get(G_OBJECT(session_), "bandwidth", &result, NULL);

    return result;
}


/// Gets the jitter for the RtpSession specified by sessionID
double RtpBin::jitter(guint sessionId) const
{
    gdouble result = 0.0;

    requestSession(sessionId);
    // FIXME: Unimplemented
    //requestSource();
    return result;
}


bool RtpBin::requestSession(guint sessionId)
{
    g_signal_emit_by_name (static_cast<gpointer>(rtpbin_), "get-internal-session", sessionId, &session_);
    return false;
}


GObject *RtpBin::gotInternalSessionCb(GstElement * /*rtpbin*/, guint session, gpointer data)
{

    LOG_DEBUG("GOT THE SESSION: " << session);
    session_ = static_cast<GObject*>(data);

    return session_;
}

