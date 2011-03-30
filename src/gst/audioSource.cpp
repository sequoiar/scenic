/* audioSource.cpp
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

#include "gstLinkable.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "jackUtils.h"
#include "pipeline.h"
#include "dv1394.h"
#include "fileSource.h"
#include "gutil/gutil.h"

#include <iostream>

/// Constructor
AudioSource::AudioSource(const Pipeline &pipeline, const AudioSourceConfig &config) :
    pipeline_(pipeline),
    config_(config),
    source_(0)
{}

/// Destructor
AudioSource::~AudioSource()
{
}

/// Constructor
AudioTestSource::AudioTestSource(const Pipeline &pipeline, const AudioSourceConfig &config) :
AudioSource(pipeline, config),
capsfilter_(pipeline_.makeElement("capsfilter", 0))
{
    gutil::initAudioCapsFilter(capsfilter_, config_.numChannels());
    GstElement *interleave = pipeline_.makeElement("interleave", NULL);

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        GstElement *source = pipeline_.makeElement(config_.source(), NULL);
        GstElement *aconv = pipeline_.makeElement("audioconvert", NULL);
        gstlinkable::link(source, aconv);
        gstlinkable::link(aconv, interleave);
    }

    gstlinkable::link(interleave, capsfilter_);
}

GstElement *AudioTestSource::srcElement()
{
    return capsfilter_;
}

const int AudioFileSource::LOOP_INFINITE = -1;

/// Constructor
AudioFileSource::AudioFileSource(Pipeline &pipeline, const AudioSourceConfig &config) :
    AudioSource(pipeline, config), BusMsgHandler(&pipeline), audioconvert_(0), loopCount_(0)
{
    if (not config_.locationExists())
        THROW_ERROR("File \"" << config_.location() << "\" does not exist");

    audioconvert_ = AudioSource::pipeline_.makeElement("audioconvert", NULL);

    GstElement * queue = FileSource::acquireAudio(pipeline, config_.location());
    gstlinkable::link(queue, audioconvert_);
}

void AudioFileSource::loop(int nTimes)
{
    if (nTimes < -1)
        THROW_ERROR("Loop setting must be either >= 0 , or -1 for infinite looping");

    loopCount_ = nTimes;
}

/// Handles EOS signal from bus, which may mean repeating playback of the file
bool AudioFileSource::handleBusMsg(_GstMessage *msg)
{
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
    {
        LOG_DEBUG("Got end of stream, here's where we should playback if needed");
        if (loopCount_ > 0 or loopCount_ == AudioFileSource::LOOP_INFINITE)
        {
            LOG_DEBUG("playback about to restart, " << loopCount_ << " times to go");
            restartPlayback();
        }
        else if (loopCount_ != 0)
            THROW_ERROR("Invalid loop count");
    }
    return false;
}


void AudioFileSource::restartPlayback()
{
    const gint64 BEGIN_TIME_NS = 0;
    BusMsgHandler::pipeline_->seekTo(BEGIN_TIME_NS); // have to specify whose pipeline reference
    if (loopCount_ > 0)  // avoids endless decrements
        loopCount_--;
}


/// Destructor
AudioFileSource::~AudioFileSource()
{
    FileSource::releaseAudio(config_.location());
}


/// Constructor
AudioAlsaSource::AudioAlsaSource(const Pipeline &pipeline, const AudioSourceConfig &config) :
    AudioSource(pipeline, config),
    capsfilter_(pipeline_.makeElement("capsfilter", 0)),
    audioconvert_(pipeline_.makeElement("audioconvert", 0))
{
    source_ = pipeline_.makeElement(config_.source(), NULL);

    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);

    gutil::initAudioCapsFilter(capsfilter_, config_.numChannels());
    gstlinkable::link(source_, audioconvert_);
    gstlinkable::link(audioconvert_, capsfilter_);
}

/// Constructor
AudioPulseSource::AudioPulseSource(const Pipeline &pipeline, const AudioSourceConfig &config) :
    AudioSource(pipeline, config),
    capsfilter_(pipeline_.makeElement("capsfilter", 0)),
    audioconvert_(pipeline_.makeElement("audioconvert", 0))
{
    source_ = pipeline_.makeElement(config_.source(), NULL);
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);

    gutil::initAudioCapsFilter(capsfilter_, config_.numChannels());
    gstlinkable::link(source_, audioconvert_);
    gstlinkable::link(audioconvert_, capsfilter_);
}

/// Constructor
AudioJackSource::AudioJackSource(const Pipeline &pipeline, const AudioSourceConfig &config) :
    AudioSource(pipeline, config),
    capsfilter_(pipeline_.makeElement("capsfilter", 0)),
    queue_(pipeline_.makeElement("queue", 0))
{
    source_ = pipeline_.makeElement(config_.source(), config_.sourceName());

    // use auto-forced connect mode if available
    g_object_set(G_OBJECT(source_), "connect", 2, NULL);

    // setup capsfilter
    gutil::initAudioCapsFilter(capsfilter_, config_.numChannels());

    if (config_.bufferTime() < Jack::safeBufferTime())
    {
        LOG_WARNING("Buffer time " << config_.bufferTime() << " is too low, using " << Jack::safeBufferTime() << " instead");
        g_object_set(G_OBJECT(source_), "buffer-time", Jack::safeBufferTime(), NULL);
    }
    else
        g_object_set(G_OBJECT(source_), "buffer-time", config_.bufferTime(), NULL);

    unsigned long long val;
    g_object_get(source_, "buffer-time", &val, NULL);
    LOG_DEBUG("Buffer time is " << val);

    gstlinkable::link(source_, capsfilter_);
    gstlinkable::link(capsfilter_, queue_);
}


void AudioJackSource::disableAutoConnect()
{
    g_assert(source_);
    g_object_set(G_OBJECT(source_), "connect", 0, NULL);
}


/// Constructor
AudioDvSource::AudioDvSource(const Pipeline &pipeline, const AudioSourceConfig &config) :
    AudioSource(pipeline, config),
    queue_(0),
    audioconvert_(0)
{
    queue_ = pipeline_.makeElement("queue", NULL);
    audioconvert_ = pipeline_.makeElement("audioconvert", NULL);

    // Now the Dv1394 will be able to link this queue to the dvdemux when the audio pad is created
    Dv1394::Instance(pipeline_)->setAudioSink(queue_);
    gstlinkable::link(queue_, audioconvert_);
}


/// Destructor
AudioDvSource::~AudioDvSource()
{
    Dv1394::Instance(pipeline_)->unsetAudioSink();
}
