/* rtpBin.cpp
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
#include <cstring>
#include "rtpBin.h"
#include "rtpPay.h"
#include "remoteConfig.h"
#include "pipeline.h"

#ifdef CONFIG_DEBUG_LOCAL
#define RTP_REPORTING 1
#endif

GstElement *RtpBin::rtpbin_ = 0;
unsigned int RtpBin::sessionCount_ = 0;
bool RtpBin::destroyed_ = false;

void RtpBin::init()
{
    // only initialize rtpbin once per process
    if (rtpbin_ == 0) 
    {
        rtpbin_ = Pipeline::Instance()->makeElement("gstrtpbin", NULL);

        // KEEP THIS LOW OR SUFFER THE CONSEQUENCES
        // rule of thumb: 2-3 times the maximum network jitter
        const int LATENCY = 3; // ms
        setLatency(LATENCY);

        // uncomment this to print stats
#if RTP_REPORTING
        g_timeout_add(5000 /* ms */, 
                static_cast<GSourceFunc>(printStatsCallback),
                static_cast<gpointer>(rtpbin_));
#endif
    }
    // DON'T USE THE DROP-ON-LATENCY SETTING, WILL CAUSE AUDIO TO DROP OUT WITH LITTLE OR NO FANFARE
}


void RtpBin::setLatency(int latency)
{
    assert(rtpbin_);
    g_object_set(G_OBJECT(rtpbin_), "latency", latency, NULL);
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


// callback to print the rtp stats 
gboolean RtpBin::printStatsCallback(gpointer data)
{
    if (destroyed_)
    {
        LOG_DEBUG("No active rtpsessions, unregistering reporting callback");
        return FALSE;
    }

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
        destroyed_ = true;
    }
}

