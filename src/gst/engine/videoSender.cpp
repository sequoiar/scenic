/* videoSender.cpp
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

#include "pipeline.h"
#include "gstLinkable.h"
#include "videoSender.h"
#include "videoSource.h"
#include "rtpPay.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "codec.h"
#include "capsParser.h"
#include "messageDispatcher.h"


/// Constructor
VideoSender::VideoSender(const VideoSourceConfig vConfig, const SenderConfig rConfig, bool capsOutOfBand) : 
    videoConfig_(vConfig), remoteConfig_(rConfig), session_(), source_(0), 
    encoder_(0), payloader_(0) 
{
    capsOutOfBand_ = capsOutOfBand;
    remoteConfig_.checkPorts();
}


/// Returns the capabilities of this VideoSender's RtpSession 
std::string VideoSender::getCaps() const
{ 
    std::string capsStr = payloader_->getCaps();
    tassert(capsStr != "");
    return capsStr;
}

VideoSender::~VideoSender()
{
    remoteConfig_.cleanupPorts();
    delete payloader_;
    delete encoder_;
    delete source_;
}


void VideoSender::init_source()
{
    tassert(source_ = videoConfig_.createSource());
    source_->init();
}


void VideoSender::init_codec()
{
    tassert(encoder_ = remoteConfig_.createVideoEncoder());
    encoder_->init();
    encoder_->setBitrate(videoConfig_.bitrate());

    gstlinkable::link(*source_, *encoder_);
}


void VideoSender::init_payloader()       
{
    tassert(payloader_ = encoder_->createPayloader());
    payloader_->init();
    if (capsOutOfBand_) // tell payloader not to send config string in header since we're sending caps
        MessageDispatcher::sendMessage("disable-send-config");
    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, remoteConfig_);
}



/** 
 * The new caps message is posted on the bus by the src pad of our udpsink, 
 * received by this videosender, and dispatched. */
bool VideoSender::handleBusMsg(GstMessage *msg)
{
    /// The stages involved are:
    /// 1) check that this is a caps-changed msg
    /// 2) FIXME: check that it's this videosender's msg by looking at the encoder name, need a more robust way of asserting that this is the intended handler
    /// 3) if the caps are not already cached, send them to the receiver
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
        if (capsOutOfBand_ or CapsParser::getVideoCaps(remoteConfig_.codec()) == "") // caps aren't already cached 
        {
            LOG_DEBUG("Sending caps for codec " << remoteConfig_.codec());
            remoteConfig_.sendMessage(std::string(newCapsStr));
            return true;
        }
        else
            return true;    // these were our caps, but we didn't need to send caps as they're already cached
    }

    return false;           // this wasn't our msg, someone else should handle it
}

