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
#include <boost/lexical_cast.hpp>
#include <string>

#include "audioSender.h"
#include "audioLevel.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "pipeline.h"
#include "codec.h"
#include "rtpPay.h"
#include "capsParser.h"

using boost::shared_ptr;

static void 
validateChannels(const AudioSourceConfig &aConfig, const SenderConfig &rConfig)
{
    using boost::lexical_cast;
    using std::string;
    if (rConfig.codec() == "mp3")
    {
        if (aConfig.numChannels() > 2)
            throw std::range_error("MP3 only accepts 1 or 2 channels, not " +
                   lexical_cast<string>(aConfig.numChannels()));
    }
#if 0
    else if (rConfig.codec() == "raw")
    {
        if (aConfig.numChannels() > 11)
            throw std::range_error("Raw only accepts 1-11 channels, not " +
                   lexical_cast<string>(aConfig.numChannels()));
    }
#endif
}

/// Constructor 
AudioSender::AudioSender(Pipeline &pipeline,
        shared_ptr<AudioSourceConfig> aConfig, 
        shared_ptr<SenderConfig> rConfig) : 
    SenderBase(rConfig),
    audioConfig_(aConfig), 
    pipeline_(pipeline),
    session_(pipeline), 
    source_(0), 
    level_(0),
    encoder_(0), 
    payloader_(0)
{
    validateChannels(*aConfig, *rConfig);
    LOG_DEBUG("Creating audio sender pipeline");
    createPipeline(pipeline);
}


bool AudioSender::checkCaps() const
{
    return CapsParser::getAudioCaps(remoteConfig_->codec(), 
            audioConfig_->numChannels(), 
            pipeline_.actualSampleRate()) != "";
}


/// Destructor 
AudioSender::~AudioSender()
{
    delete payloader_;
    delete encoder_;
    delete level_;
    delete source_;
}

void AudioSender::createSource(Pipeline &pipeline)
{
    tassert(source_ = audioConfig_->createSource(pipeline));
    level_ = audioConfig_->createLevel(pipeline);
    if (level_ != 0)
            gstlinkable::link(*source_, *level_);
}

void AudioSender::createCodec(Pipeline &pipeline)
{
    tassert(encoder_ = remoteConfig_->createAudioEncoder(pipeline, audioConfig_->bitrate(), audioConfig_->quality()));
    if (level_ != 0)
        gstlinkable::link(*level_, *encoder_);
    else
        gstlinkable::link(*source_, *encoder_);
}


void AudioSender::createPayloader()   
{
    tassert(payloader_ = encoder_->createPayloader());

    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_, *remoteConfig_);   
}

