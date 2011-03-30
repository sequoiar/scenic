/* audioSender.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
#include "gstLinkable.h"

using std::tr1::shared_ptr;

static void 
validateChannels(const AudioSourceConfig &aConfig, const SenderConfig &rConfig)
{
    using boost::lexical_cast;
    using std::string;
    if (rConfig.codec() == "mp3")
    {
        if (aConfig.numChannels() > 2)
            throw std::runtime_error("MP3 only accepts 1 or 2 channels, not " +
                   lexical_cast<string>(aConfig.numChannels()));
    }
}

/// Constructor 
AudioSender::AudioSender(Pipeline &pipeline,
        const shared_ptr<AudioSourceConfig> &aConfig,
        const shared_ptr<SenderConfig> &rConfig) :
    SenderBase(rConfig),
    audioConfig_(aConfig),
    pipeline_(pipeline),
    session_(pipeline),
    source_(),
    level_(),
    encoder_(),
    payloader_()
{
    validateChannels(*aConfig, *rConfig);
    LOG_DEBUG("Creating audio sender pipeline");
    createPipeline(pipeline);
}

void AudioSender::createSource(Pipeline &pipeline)
{
    source_.reset(audioConfig_->createSource(pipeline));
    assert(source_);
    level_.reset(audioConfig_->createLevel(pipeline));
    gstlinkable::link(*source_, *level_);
}

void AudioSender::createCodec(Pipeline &pipeline)
{
    encoder_.reset(remoteConfig_->createAudioEncoder(pipeline, audioConfig_->bitrate(), audioConfig_->quality()));
    assert(encoder_);
    gstlinkable::link(*level_, *encoder_);
}


void AudioSender::createPayloader()   
{
    payloader_.reset(encoder_->createPayloader());
    assert(payloader_);

    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_.get(), *remoteConfig_);
}

