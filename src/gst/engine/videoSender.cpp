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

#include "pipeline.h"
#include "gstLinkable.h"
#include "videoSender.h"
#include "videoSource.h"
#include "rtpPay.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "codec.h"

/// Constructor
VideoSender::VideoSender(const VideoSourceConfig vConfig, const SenderConfig rConfig) : 
    videoConfig_(vConfig), remoteConfig_(rConfig), session_(), source_(0), 
    encoder_(0), payloader_(0) 
{
    remoteConfig_.checkPorts();
}


/// Returns the capabilities of this VideoSender's RtpSession 
std::string VideoSender::getCaps() const
{ 
    std::string capsStr = session_.getCaps();
    tassert(capsStr != "");
    return capsStr;
}

VideoSender::~VideoSender()
{
    remoteConfig_.cleanupPorts();
    delete payloader_;
    delete encoder_;
    delete source_;
}


void VideoSender::init_source()
{
    tassert(source_ = videoConfig_.createSource());
    source_->init();
}


void VideoSender::init_codec()
{
    tassert(encoder_ = remoteConfig_.createVideoEncoder());
    if (videoConfig_.doDeinterlace())
        encoder_->doDeinterlace();
    encoder_->init();
    encoder_->setBitrate(videoConfig_.bitrate());
    if (not encoder_->supportsCaps(source_->srcCaps()))
    {
        LOG_DEBUG("Encoder does not support " << source_->srcCaps() << ", setting to " <<
                encoder_->supportedCaps());
        source_->setCapsFilter(encoder_->supportedCaps());
    }

    gstlinkable::link(*source_, *encoder_);
}


void VideoSender::init_payloader()       
{
    tassert(payloader_ = encoder_->createPayloader());
    payloader_->init();
    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, remoteConfig_);
}

