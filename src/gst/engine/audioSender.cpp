/* audioSender.cpp
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

#include "audioSender.h"
#include "audioSource.h"
#include "audioLevel.h"
#include "pipeline.h"
#include "codec.h"
#include "rtpPay.h"
#include "capsParser.h"
#include "playback.h"


/// Constructor 
AudioSender::AudioSender(const AudioSourceConfig aConfig, const SenderConfig rConfig, bool capsOutOfBand) : 
    audioConfig_(aConfig), 
    remoteConfig_(rConfig), 
    session_(), 
    source_(0), 
    //level_(), 
    encoder_(0), 
    payloader_(0)
{
    capsOutOfBand_ = capsOutOfBand;

    remoteConfig_.checkPorts();
    if (remoteConfig_.codec() == "mp3")
        if (audioConfig_.numChannels() < 1 or audioConfig_.numChannels() > 2)
            THROW_CRITICAL("MP3 only accepts 1 or 2 channels, not " << audioConfig_.numChannels());
}

/// Destructor 
AudioSender::~AudioSender()
{
    remoteConfig_.cleanupPorts();
    delete payloader_;
    delete encoder_;
    delete source_;
}

/// Returns the capabilities of this AudioSender's RtpSession 
std::string AudioSender::getCaps() const
{ 
    std::string capsStr = payloader_->getCaps();
    tassert(capsStr != "");
    return capsStr;
}

void AudioSender::init_source()
{
    tassert(source_ = audioConfig_.createSource());
    source_->init();
    //init_level();
}


#if 0
void AudioSender::init_level()
{
    level_.init();
    gstlinkable::link(*source_, level_);
}
#endif


void AudioSender::init_codec()
{
    tassert(encoder_ = remoteConfig_.createAudioEncoder());
    encoder_->init();

    //gstlinkable::link(level_, *encoder_);
    gstlinkable::link(*source_, *encoder_);
}


void AudioSender::init_payloader()   
{
    tassert(payloader_ = encoder_->createPayloader());
    payloader_->init();

    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, remoteConfig_);   
}




/** 
 * The new caps message is posted on the bus by the src pad of our udpsink, 
 * received by this audiosender, and sent to our other host if needed. */
bool AudioSender::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);

    if (std::string(name) == "caps-changed") 
    {   
        LOG_DEBUG("Got caps changed signal");
        // this is our msg
        const gchar *newCapsStr = gst_structure_get_string(s, "caps");
        tassert(newCapsStr);
        std::string str(newCapsStr);

        GstStructure *structure = gst_caps_get_structure(gst_caps_from_string(str.c_str()), 0);
        const GValue *encodingStr = gst_structure_get_value(structure, "encoding-name");
        std::string encodingName(g_value_get_string(encodingStr));

        if (!RemoteConfig::capsMatchCodec(encodingName, remoteConfig_.codec()))
                return false;   // not our caps, ignore it
        else if (capsOutOfBand_ or CapsParser::getAudioCaps(remoteConfig_.codec(), 
                    audioConfig_.numChannels(), playback::sampleRate()) == "") 
        { 
            LOG_DEBUG("Sending caps for codec " << remoteConfig_.codec());
            // this profile needs the caps to be sent
            remoteConfig_.setMessage(std::string(newCapsStr));
            g_timeout_add(500 /* ms */, static_cast<GSourceFunc>(SenderConfig::sendMessage), &remoteConfig_);
            return true;
        }
        else
            return true;       // was our caps, but we don't need to send caps for it
    }

    return false;           // this wasn't our msg, someone else should handle it
}

