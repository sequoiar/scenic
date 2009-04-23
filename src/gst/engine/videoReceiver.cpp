/* videoReceiver.cpp
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
#include "mediaBase.h"
#include "gstLinkable.h"
#include "videoReceiver.h"
#include "videoSink.h"
#include "codec.h"
#include "rtpPay.h"

    
VideoReceiver::VideoReceiver(const VideoSinkConfig vConfig, const ReceiverConfig rConfig) : 
    videoConfig_(vConfig), 
    remoteConfig_(rConfig), 
    session_(), 
    depayloader_(0), 
    decoder_(0), 
    sink_(0), 
    gotCaps_(false) 
{
    assert(remoteConfig_.hasCodec()); 
    remoteConfig_.checkPorts();
}

VideoReceiver::~VideoReceiver()
{
    remoteConfig_.cleanupPorts();
    delete sink_;
    delete depayloader_;
    delete decoder_;
}

void VideoReceiver::makeFullscreen()
{
    sink_->makeFullscreen();
}

void VideoReceiver::init_codec()
{
    assert(decoder_ = remoteConfig_.createVideoDecoder());
    decoder_->init();
}


void VideoReceiver::init_depayloader()
{
    assert(depayloader_ = decoder_->createDepayloader());
    depayloader_->init();

    gstlinkable::link(*depayloader_, *decoder_);

    session_.add(depayloader_, remoteConfig_);
}


void VideoReceiver::init_sink()
{
    assert(sink_ = videoConfig_.createSink());
    sink_->init();
    gstlinkable::link(*decoder_, *sink_);
    setCaps();
    assert(gotCaps_);
    assert(remoteConfig_.capsMatchCodec()); 
}

/// Used to set this VideoReceiver's RtpReceiver's caps 
void VideoReceiver::setCaps() 
{ 
    //session_.setCaps(decoder_->getCaps()); // initialize to default
    session_.setCaps(remoteConfig_.caps()); 
    gotCaps_ = true;
}

