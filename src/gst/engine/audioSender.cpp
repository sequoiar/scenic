
// audioSender.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//
//
// TODO: Client code should just call this with some kind of parameter object which
// specifies number of channels, how to compress it (if at all), and host and port info.

#include <cassert>
#include <iostream>
#include <sstream>
#include "logWriter.h"

#include "audioSender.h"
#include "audioSource.h"
#include "audioLevel.h"
#include "pipeline.h"
#include "codec.h"
#include "rtpPay.h"
#include "hostIP.h"

/// Constructor 
AudioSender::AudioSender(const AudioSourceConfig aConfig, const SenderConfig rConfig) : 
    audioConfig_(aConfig), 
    remoteConfig_(rConfig), 
    session_(), 
    source_(0), 
    level_(), 
    encoder_(0), 
    payloader_(0)
{}

/// Destructor 
AudioSender::~AudioSender()
{
    delete payloader_;
    delete encoder_;
    delete source_;
}

/// Returns the capabilities of this AudioSender's RtpSession 
std::string AudioSender::getCaps() const
{ 
    return session_.getCaps(); 
}

void AudioSender::init_source()
{
    assert(source_ = audioConfig_.createSource());
    source_->init();
    init_level();
}


void AudioSender::init_level()
{
    level_.init();
    gstlinkable::link(*source_, level_);
}


void AudioSender::init_codec()
{
    assert(encoder_ = remoteConfig_.createAudioEncoder());
    encoder_->init();

    gstlinkable::link(level_, *encoder_);
}


void AudioSender::init_payloader()   
{
    assert(payloader_ = encoder_->createPayloader());
    payloader_->init();

    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, remoteConfig_);   // FIXME: session should take RtpPay pointer
}

