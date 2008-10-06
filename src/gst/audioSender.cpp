
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

AudioSender::~AudioSender()
{
    assert(stop());
    delete payloader_;
    delete encoder_;
    delete source_;
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
    GstLinkable::link(*source_, level_);
}


void AudioSender::init_codec()
{
        assert(encoder_ = remoteConfig_.createEncoder());
        encoder_->init();

        GstLinkable::link(level_, *encoder_);
}


void AudioSender::init_payloader()   
{
     assert(payloader_ = encoder_->createPayloader());
     payloader_->init();
     
     GstLinkable::link(*encoder_, *payloader_);
     session_.add(payloader_, remoteConfig_);   // FIXME: session should take RtpPay pointer
}

// CAPS can only be sent after this is started
bool AudioSender::start()
{
    GstBase::start();

    std::stringstream logstr;
    logstr << "Sending audio to host " << remoteConfig_.remoteHost() << " on port " << remoteConfig_.port() << std::endl;
    LOG_DEBUG(logstr.str());

    pipeline_.wait_until_playing();
    session_.checkSampleRate();

    return true;
}

