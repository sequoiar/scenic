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


using boost::shared_ptr;

/// Constructor 
AudioSender::AudioSender(shared_ptr<AudioSourceConfig> aConfig, 
        shared_ptr<SenderConfig> rConfig) : 
    SenderBase(rConfig),
    audioConfig_(aConfig), 
    session_(), 
    source_(0), 
    //level_(), 
    encoder_(0), 
    payloader_(0)
{
    if (remoteConfig_->codec() == "mp3")
        if (audioConfig_->numChannels() < 1 or audioConfig_->numChannels() > 2)
            THROW_CRITICAL("MP3 only accepts 1 or 2 channels, not " << audioConfig_->numChannels());
}


bool AudioSender::checkCaps() const
{
    return CapsParser::getAudioCaps(remoteConfig_->codec(), 
            audioConfig_->numChannels(), 
            playback::sampleRate()) != "";
}


/// Destructor 
AudioSender::~AudioSender()
{
    delete payloader_;
    delete encoder_;
    delete source_;
}

void AudioSender::init_source()
{
    tassert(source_ = audioConfig_->createSource());
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
    tassert(encoder_ = remoteConfig_->createAudioEncoder());

    //gstlinkable::link(level_, *encoder_);
    gstlinkable::link(*source_, *encoder_);
}


void AudioSender::init_payloader()   
{
    tassert(payloader_ = encoder_->createPayloader());

    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, *remoteConfig_);   
}

