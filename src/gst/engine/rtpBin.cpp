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

// for posting 
#include "mapMsg.h"
#include <sstream>

#ifdef CONFIG_DEBUG_LOCAL
#define RTP_REPORTING 0
#endif

GstElement *RtpBin::rtpbin_ = 0;
int RtpBin::sessionCount_ = 0;
bool RtpBin::destroyed_ = false;


std::map<int, std::string> RtpBin::sessionNames_;

void RtpBin::init()
{
    // only initialize rtpbin once per process
    if (rtpbin_ == 0) 
    {
        rtpbin_ = Pipeline::Instance()->makeElement("gstrtpbin", NULL);

        // uncomment this to print stats
        g_timeout_add(REPORTING_PERIOD_MS /* ms */, 
                static_cast<GSourceFunc>(printStatsCallback),
                static_cast<gpointer>(this));
    }
    // DON'T USE THE DROP-ON-LATENCY SETTING, WILL CAUSE AUDIO TO DROP OUT WITH LITTLE OR NO FANFARE
}


void RtpBin::printStatsVal(const std::string &idStr, const char *key, const std::string &type, const std::string &formatStr, GstStructure *stats)
{
    MapMsg mapMsg;
    std::stringstream paramStr;
    if (type == "guint64")
    {
        guint64 val = g_value_get_uint64(gst_structure_get_value(stats, key));
        paramStr << formatStr << val;
    }
    else if (type == "guint32")
    {
        guint32 val = g_value_get_uint(gst_structure_get_value(stats, key));
        paramStr << formatStr << val;
    }
    else if (type == "gint32")
    {
        gint32 val = g_value_get_int(gst_structure_get_value(stats, key));
        paramStr << formatStr << val;
    }
    else
        THROW_ERROR("Unknown type");

    mapMsg["stats"] = idStr + paramStr.str();
    LOG_DEBUG(mapMsg["stats"]);
    mapMsg.post();
}


void RtpBin::parseSourceStats(GObject * source, int sessionId, RtpBin *context)
{
    GstStructure *stats;
    std::stringstream idStr;
    idStr << sessionNames_[sessionId] << "_" << sessionId;
    MapMsg mapMsg("rtp");

    // get the source stats
    g_object_get(source, "stats", &stats, NULL);
    
    /* simply dump the stats structure */
    // gchar *str = gst_structure_to_string (stats);
    // g_print ("source stats: %s\n", str);

    context->subParseSourceStats(idStr.str(), stats);  // let our subclasses parse the stats

    // free structures
    gst_structure_free (stats);
    //g_free (str);
}


// callback to print the rtp stats 
gboolean RtpBin::printStatsCallback(gpointer data)
{
    if (sessionCount_ <= 0) // no sessions o print yet
        return TRUE; 

    if (destroyed_)
    {
        LOG_DEBUG("No active rtpsessions, unregistering reporting callback");
        return FALSE;
    }

    GObject *session;
    GValueArray *arrayOfSources;
    GValue *val;
    guint i;

    RtpBin *context = static_cast<RtpBin*>(data);

    // get sessions
    for (int sessionId = 0; sessionId < sessionCount_; ++sessionId)
    {
        g_signal_emit_by_name(context->rtpbin_, "get-internal-session", sessionId, &session);

        // parse stats of all the sources in the session, this include the internal source
        g_object_get(session, "sources", &arrayOfSources, NULL);

        for (i = 0; i < arrayOfSources->n_values; ++i)
        {
            GObject *source;

            val = g_value_array_get_nth(arrayOfSources, i);
            source = static_cast<GObject*>(g_value_get_object(val));

            parseSourceStats(source, sessionId, context);
        }
        g_value_array_free(arrayOfSources);

        g_object_unref(session);
    }
    return TRUE;
}



const char *RtpBin::padStr(const char *padName)
{
    tassert(sessionCount_ > 0);  // we have a session going
    std::string result(padName);
    std::stringstream istream;

    istream << sessionId_;       // 0-based
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
        tassert(sessionCount_ == 0);
        Pipeline::Instance()->remove(&rtpbin_);
        rtpbin_ = 0;
        destroyed_ = true;
    }
}

