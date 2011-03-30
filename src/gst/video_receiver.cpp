/* video_receiver.cpp
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
#include "video_receiver.h"
#include "video_scale.h"
#include "video_config.h"
#include "remote_config.h"
#include "video_flip.h"
#include "text_overlay.h"
#include "video_sink.h"
#include "codec.h"
#include "rtp_pay.h"

using std::tr1::shared_ptr;

VideoReceiver::VideoReceiver(Pipeline &pipeline,
        const shared_ptr<VideoSinkConfig> &vConfig,
        const shared_ptr<ReceiverConfig> &rConfig) :
    videoConfig_(vConfig),
    remoteConfig_(rConfig),
    session_(pipeline),
    depayloader_(),
    decoder_(),
    textoverlay_(),
    videoscale_(),
    videoflip_(),
    sink_(),
    gotCaps_(false)
{
    assert(remoteConfig_->hasCodec());
    remoteConfig_->checkPorts();
    createPipeline(pipeline);
}

VideoReceiver::~VideoReceiver()
{
    remoteConfig_->cleanupPorts();
}


void VideoReceiver::createCodec(Pipeline &pipeline)
{
    decoder_.reset(remoteConfig_->createVideoDecoder(pipeline, videoConfig_->doDeinterlace()));
    assert(decoder_);
}


void VideoReceiver::createDepayloader()
{
    depayloader_.reset(decoder_->createDepayloader());
    assert(depayloader_);

    gstlinkable::link(*depayloader_, *decoder_);

    session_.add(depayloader_.get(), *remoteConfig_);
}


void VideoReceiver::createSink(Pipeline &pipeline)
{
    videoscale_.reset(videoConfig_->createVideoScale(pipeline));
    assert(videoscale_);

    textoverlay_.reset(videoConfig_->createTextOverlay(pipeline));

    videoflip_.reset(videoConfig_->createVideoFlip(pipeline));
    assert(videoflip_);
    sink_.reset(videoConfig_->createSink(pipeline));
    assert(sink_);

    gstlinkable::link(*decoder_, *textoverlay_);
    gstlinkable::link(*textoverlay_, *videoscale_);
    gstlinkable::link(*videoscale_, *videoflip_);
    gstlinkable::link(*videoflip_, *sink_);

    setCaps();
    assert(gotCaps_);
    if (not remoteConfig_->capsMatchCodec())
        THROW_CRITICAL("Incoming caps don't match expected codec " << remoteConfig_->codec());
    decoder_->adjustJitterBuffer(); // increase jitterbuffer as needed
}

/// Used to set this VideoReceiver's RtpReceiver's caps
void VideoReceiver::setCaps()
{
    remoteConfig_->receiveCaps();
    session_.setCaps(remoteConfig_->caps());
    gotCaps_ = true;
}

