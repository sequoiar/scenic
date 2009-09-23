/* mediaBase.cpp
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

#include "mediaBase.h"
#include <gst/gst.h>


SenderBase::SenderBase(SenderConfig rConfig, bool capsOutOfband) : 
    remoteConfig_(rConfig), capsOutOfBand_(capsOutOfband), initialized_(false) 
{
    remoteConfig_.checkPorts();
}

void SenderBase::init()  // template method
{
    capsOutOfBand_ = capsOutOfBand_ or !capsAreCached();

    // these methods are defined in subclasses
    tassert(!initialized_);
    init_source();
    init_codec();
    init_payloader();
    initialized_ = true;
}


/** 
 * The new caps message is posted on the bus by the src pad of our udpsink, 
 * received by this audiosender, and sent to our other host if needed. */
bool SenderBase::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);

    if (std::string(name) == "caps-changed") 
    {   
        // this is our msg
        const gchar *newCapsStr = gst_structure_get_string(s, "caps");
        tassert(newCapsStr);
        std::string str(newCapsStr);

        GstStructure *structure = gst_caps_get_structure(gst_caps_from_string(str.c_str()), 0);
        const GValue *encodingStr = gst_structure_get_value(structure, "encoding-name");
        std::string encodingName(g_value_get_string(encodingStr));

        if (!RemoteConfig::capsMatchCodec(encodingName, remoteConfig_.codec()))
            return false;   // not our caps, ignore it
        else if (capsOutOfBand_) 
        { 
            LOG_DEBUG("Sending caps for codec " << remoteConfig_.codec());

            remoteConfig_.setMessage(std::string(newCapsStr));
            enum {MESSAGE_SEND_TIMEOUT = 500};
            g_timeout_add(MESSAGE_SEND_TIMEOUT /* ms */, static_cast<GSourceFunc>(SenderConfig::sendMessage), &remoteConfig_);
            return true;
        }
        else
            return true;       // was our caps, but we don't need to send caps for it
    }

    return false;           // this wasn't our msg, someone else should handle it
}


SenderBase::~SenderBase()
{
    remoteConfig_.cleanupPorts();
}


void ReceiverBase::init()  // template method
{
    // these methods are defined in subclasses
    tassert(!initialized_);
    init_codec();
    init_depayloader();
    init_sink();
    initialized_ = true;
}

