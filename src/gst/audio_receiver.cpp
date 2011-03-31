/*
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

#include "pipeline.h"
#include "gst_linkable.h"
#include "audio_receiver.h"
#include "audio_config.h"
#include "remote_config.h"
#include "rtp_pay.h"
#include "codec.h"
#include "audio_level.h"
#include "audio_sink.h"

#include <tr1/memory>

using std::tr1::shared_ptr;

/** Constructor parameterized by an AudioSinkConfig
 * and a ReceiverConfig */
AudioReceiver::AudioReceiver(Pipeline &pipeline,
        const shared_ptr<AudioSinkConfig> &aConfig,
        const shared_ptr<ReceiverConfig> &rConfig) :
    audioConfig_(aConfig),
    remoteConfig_(rConfig),
    session_(pipeline),
    gotCaps_(false),
    depayloader_(),
    decoder_(),
    level_(),
    sink_()
{
    std::string title("Receiving from ");
    title += rConfig->remoteHost();
    level_.reset(audioConfig_->createLevel(pipeline, title));
    assert(remoteConfig_->hasCodec());
    remoteConfig_->checkPorts();
    createPipeline(pipeline);
}

/// Destructor
AudioReceiver::~AudioReceiver()
{
    remoteConfig_->cleanupPorts();
}


void AudioReceiver::createCodec(Pipeline &pipeline)
{
    decoder_.reset(remoteConfig_->createAudioDecoder(pipeline, audioConfig_->numChannels()));
    assert(decoder_);
    gstlinkable::link(*decoder_, *level_);
}


void AudioReceiver::createDepayloader()
{
    depayloader_.reset(decoder_->createDepayloader());
    assert(depayloader_);
    gstlinkable::link(*depayloader_, *decoder_);
    session_.add(depayloader_.get(), *remoteConfig_);
}


void AudioReceiver::createSink(Pipeline &pipeline)
{
    sink_.reset(audioConfig_->createSink(pipeline));
    assert(sink_);
    // Add an audioresample in case we're being sent audio at a different samplerate
    GstElement *audioresample = pipeline.makeElement("audioresample", NULL);
    gstlinkable::link(*level_, audioresample);

    gstlinkable::link(audioresample, *sink_);
    setCaps();
    assert(gotCaps_);
    if (not remoteConfig_->capsMatchCodec())
        THROW_CRITICAL("Incoming caps don't match expected codec " << remoteConfig_->codec());

    if (decoder_->adjustsBufferTime())
        sink_->adjustBufferTime(decoder_->minimumBufferTime()); // increase jitterbuffer as needed
}

/// Used to set this AudioReceiver's RtpReceiver's caps
void AudioReceiver::setCaps()
{
    remoteConfig_->receiveCaps(); // this call will block
    session_.setCaps(remoteConfig_->caps());
    gotCaps_ = true;
}

