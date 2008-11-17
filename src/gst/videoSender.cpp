
// videoSender.cpp
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

#include <cassert>

#include "pipeline.h"
#include "gstLinkable.h"
#include "videoSender.h"
#include "videoSource.h"
#include "rtpPay.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "codec.h"
#include "logWriter.h"


VideoSender::~VideoSender()
{
    stop();
    delete payloader_;
    delete encoder_;
    delete source_;
}


void VideoSender::init_source()
{
    assert(source_ = videoConfig_.createSource());
    source_->init();
}


void VideoSender::init_codec()
{
        assert(encoder_ = remoteConfig_.createEncoder());
        encoder_->init();
        gstlinkable::link(*source_, *encoder_);// FIXME: this shouldn't happen for VideoFileSource
}


void VideoSender::init_payloader()       
{
        assert(payloader_ = encoder_->createPayloader());
        payloader_->init();
        gstlinkable::link(*encoder_, *payloader_);
        session_.add(payloader_, remoteConfig_);
}


void VideoSender::start()
{
    GstBase::start();
    //pipeline_.wait_until_playing();
}


