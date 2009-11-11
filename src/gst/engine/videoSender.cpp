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
#include "mapMsg.h"

#include "pipeline.h"
#include "gstLinkable.h"
#include "videoSender.h"
#include "videoSource.h"
#include "rtpPay.h"
#include "videoConfig.h"
#include "codec.h"
#include "capsParser.h"
#include "messageDispatcher.h"

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

/// Constructor
VideoSender::VideoSender(shared_ptr<VideoSourceConfig> vConfig, 
        shared_ptr<SenderConfig> rConfig) : 
    SenderBase(rConfig), videoConfig_(vConfig), session_(), source_(0), 
    encoder_(0), payloader_(0) 
{}

bool VideoSender::checkCaps() const
{
    return CapsParser::getVideoCaps(remoteConfig_->codec()) != ""; 
}

VideoSender::~VideoSender()
{
    delete payloader_;
    delete encoder_;
    delete source_;
}


void VideoSender::init_source()
{
    tassert(source_ = videoConfig_->createSource());
    source_->init();
}


void VideoSender::init_codec()
{
    MapMsg settings;
    settings["bitrate"] = videoConfig_->bitrate();
    settings["quality"] = videoConfig_->quality();
    tassert(encoder_ = remoteConfig_->createVideoEncoder(settings));

    gstlinkable::link(*source_, *encoder_);
}


void VideoSender::init_payloader()       
{
    tassert(payloader_ = encoder_->createPayloader());
    // tell rtpmp4vpay not to send config string in header since we're sending caps
    if (remoteConfig_->capsOutOfBand() and remoteConfig_->codec() == "mpeg4") 
        MessageDispatcher::sendMessage("disable-send-config");
    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, *remoteConfig_);
}

