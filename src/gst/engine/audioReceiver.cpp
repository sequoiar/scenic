/* audioReceiver.cpp
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
#include "audioReceiver.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "rtpPay.h"
#include "codec.h"
#include "audioSink.h"

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

/** Constructor parameterized by an AudioSinkConfig 
 * and a ReceiverConfig */
AudioReceiver::AudioReceiver(shared_ptr<AudioSinkConfig> aConfig, 
        shared_ptr<ReceiverConfig> rConfig) : 
    audioConfig_(aConfig), 
    remoteConfig_(rConfig), 
    session_(), 
    gotCaps_(false),
    depayloader_(0), 
    decoder_(0), 
    sink_(0)
{ 
    tassert(remoteConfig_->hasCodec()); 
    remoteConfig_->checkPorts();
    createPipeline();
}

/// Destructor 
AudioReceiver::~AudioReceiver()
{
    remoteConfig_->cleanupPorts();
    delete sink_;
    delete decoder_;
    delete depayloader_;
}


void AudioReceiver::createCodec()
{
    tassert(decoder_ = remoteConfig_->createAudioDecoder());
}


void AudioReceiver::createDepayloader()
{
    tassert(depayloader_ = decoder_->createDepayloader());
    gstlinkable::link(*depayloader_, *decoder_);
    session_.add(depayloader_, *remoteConfig_);
}


void AudioReceiver::createSink()
{
    tassert(sink_ = audioConfig_->createSink());
    gstlinkable::link(*decoder_, *sink_);   
    setCaps();
    tassert(gotCaps_);
    tassert(remoteConfig_->capsMatchCodec()); 
    if (decoder_->adjustsBufferTime())
        sink_->adjustBufferTime(decoder_->minimumBufferTime()); // increase jitterbuffer as needed
}

/// Used to set this AudioReceiver's RtpReceiver's caps 
void AudioReceiver::setCaps() 
{ 
    session_.setCaps(remoteConfig_->caps()); 
    gotCaps_ = true;
}


