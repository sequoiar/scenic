/* videoReceiver.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

VideoReceiver::~VideoReceiver()
{
    delete sink_;
    delete depayloader_;
    delete decoder_;
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
    
    session_.set_caps(decoder_->getCaps());
}


void VideoReceiver::init_sink()
{
    assert(sink_ = videoConfig_.createSink());
    sink_->init();
    gstlinkable::link(*decoder_, *sink_);
}

