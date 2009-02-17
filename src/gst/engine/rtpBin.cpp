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

#ifdef CONFIG_DEBUG_LOCAL
#define RTP_REPORTING 0
#endif

GstElement *RtpBin::rtpbin_ = 0;
unsigned int RtpBin::sessionCount_ = 0;

void RtpBin::init()
{
    // only initialize rtpbin once per process
    if (rtpbin_ == 0) 
        rtpbin_ = Pipeline::Instance()->makeElement("gstrtpbin", NULL);
    
    // KEEP THIS LOW OR SUFFER THE CONSEQUENCES
    g_object_set(G_OBJECT(rtpbin_), "latency", 20, NULL);
    
    // uncomment this to print stats
#if RTP_REPORTING
    g_timeout_add(1000 /* ms */, 
                  static_cast<GSourceFunc>(printStatsCallback),
                  static_cast<gpointer>(rtpbin_));
#endif
}


void RtpBin::parseSourceStats(GObject * source, int sessionId)
{
    GstStructure *stats;

    // get the source stats
    g_object_get(source, "stats", &stats, NULL);

    const GValue *val = gst_structure_get_value(stats, "internal");
    if (g_value_get_boolean(val))
        return; // we don't care about internal sources

    g_print("SESSION %d:\n", sessionId);
    guint jitter = g_value_get_uint(gst_structure_get_value(stats, "rb-jitter"));
    g_print("JITTER: %u\n", jitter);
    int packetLoss = g_value_get_int(gst_structure_get_value(stats, "rb-packetslost"));
    g_print("PACKETS LOST: %d\n", packetLoss);
}

void RtpBin::printSourceStats(GObject * source)
{
    GstStructure *stats;
    gchar *str;

    // get the source stats
    g_object_get(source, "stats", &stats, NULL);

    // dump the whole structure
    str = gst_structure_to_string(stats);
    g_print("source stats: %s\n", str);

    gst_structure_free(stats);
    g_free(str);
}


// callback to print the rtp stats 
gboolean RtpBin::printStatsCallback(gpointer data)
{
    GObject *session;
    GValueArray *arrayOfSources;
    GValue *val;
    guint i;
    
    GstElement *rtpbin = static_cast<GstElement*>(data);

    g_print("/*----------------------------------------------*/\n");  
    // get sessions
    for (unsigned int sessionId = 0; sessionId < sessionCount_; ++sessionId)
    {
        g_signal_emit_by_name(rtpbin, "get-internal-session", sessionId, &session);

        // print all the sources in the session, this include the internal source
        g_object_get(session, "sources", &arrayOfSources, NULL);

        for (i = 0; i < arrayOfSources->n_values; ++i)
        {
            GObject *source;

            val = g_value_array_get_nth(arrayOfSources, i);
            source = static_cast<GObject*>(g_value_get_object(val));

            parseSourceStats(source, sessionId);
        }
        g_value_array_free(arrayOfSources);

        g_object_unref(session);

    }
    return TRUE;
}


/// set drop-on-latency to TRUE, needs to be called upon creation of jitterbuffers, via a signal handler.
/// No visible effect, so not used. 

int RtpBin::dropOnLatency(gpointer data)
{
    RtpBin *context = static_cast<RtpBin*>(data);
    for (unsigned int sessionId = 0; sessionId < sessionCount_; ++sessionId)
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


const char *RtpBin::padStr(const char *padName)
{
    assert(sessionCount_ > 0);
    std::string result(padName);
    std::stringstream istream;

    istream << sessionCount_ - 1;        // 0-based
    result = result + istream.str();
    return result.c_str();
}


RtpBin::~RtpBin()
{
    Pipeline::Instance()->remove(&rtcp_sender_);
    Pipeline::Instance()->remove(&rtcp_receiver_);

    --sessionCount_;
    if (sessionCount_ <= 0) // destroy if no streams are present
    {
        assert(sessionCount_ == 0);
        Pipeline::Instance()->remove(&rtpbin_);
        rtpbin_ = 0;
    }
}

