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

/// Constructor 
AudioSender::AudioSender(const AudioSourceConfig aConfig, const SenderConfig rConfig) : 
    audioConfig_(aConfig), 
    remoteConfig_(rConfig), 
    session_(), 
    source_(0), 
    //level_(), 
    encoder_(0), 
    payloader_(0)
{
    remoteConfig_.checkPorts();
    if (remoteConfig_.codec() == "mp3")
    {
        if (audioConfig_.numChannels() < 1 or audioConfig_.numChannels() > 2)
            THROW_CRITICAL("MP3 only accepts 1 or 2 channels, not " << audioConfig_.numChannels());
    }
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
    std::string capsStr = session_.getCaps();
    assert(capsStr != "");
    return capsStr;
}

void AudioSender::init_source()
{
    assert(source_ = audioConfig_.createSource());
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
    assert(encoder_ = remoteConfig_.createAudioEncoder());
    encoder_->init();

    //gstlinkable::link(level_, *encoder_);
    gstlinkable::link(*source_, *encoder_);
}


void AudioSender::init_payloader()   
{
    assert(payloader_ = encoder_->createPayloader());
    payloader_->init();

    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, remoteConfig_);   
}

