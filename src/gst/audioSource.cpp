/* audioSource.cpp
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

#include "gstLinkable.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "jackUtils.h"
#include "pipeline.h"
#include "dv1394.h"
#include "fileSource.h"

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


std::string AudioSource::getCapsFilterCapsString()
{
    // force proper number of channels on output
    std::ostringstream capsStr;
    capsStr << "audio/x-raw-int, channels=" << config_.numChannels() 
        << ", rate=" << pipeline_.actualSampleRate();
    LOG_DEBUG("Audiosource caps = " << capsStr.str());
    return capsStr.str();
}


void AudioSource::initCapsFilter(GstElement* &aconv, GstElement* &capsFilter)
{
    // setup capsfilter
    GstCaps *caps = 0;
    caps = gst_caps_from_string(getCapsFilterCapsString().c_str());
    g_assert(caps);
    capsFilter = pipeline_.makeElement("capsfilter", NULL);
    aconv = pipeline_.makeElement("audioconvert", NULL);
    g_object_set(G_OBJECT(capsFilter), "caps", caps, NULL);

    gst_caps_unref(caps);
    
    gstlinkable::link(source_, aconv);
    gstlinkable::link(aconv, capsFilter);
}


/// Constructor 
InterleavedAudioSource::InterleavedAudioSource(const Pipeline &pipeline, const AudioSourceConfig &config) : 
    AudioSource(pipeline, config), 
    interleave_(pipeline, config_), 
    sources_(), 
    aconvs_()
{
    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        sources_.push_back(pipeline_.makeElement(config_.source(), NULL));
        aconvs_.push_back(pipeline_.makeElement("audioconvert", NULL));
    }
    gstlinkable::link(sources_, aconvs_);
    gstlinkable::link(aconvs_, interleave_);
}

/// Constructor 
AudioTestSource::AudioTestSource(const Pipeline &pipeline, const AudioSourceConfig &config) : 
    InterleavedAudioSource(pipeline, config), 
    frequencies_(),
    offset_(0) 
{
    frequencies_.push_back(std::vector<double>()); // two rows
    frequencies_.push_back(std::vector<double>());
    for (int channel = 0; channel < config_.numChannels(); ++channel)
    {
        frequencies_[0].push_back((100 * channel) + 200);
        frequencies_[1].push_back(frequencies_[0].back() + 100);
    }

    gstlinkable::GstIter src;

    int channelIdx = 0;

    GstCaps *caps = gst_caps_new_simple("audio/x-raw-int", "endianness", G_TYPE_INT, 1234, "signed", 
            G_TYPE_BOOLEAN, TRUE, "width", G_TYPE_INT, 32, "depth", G_TYPE_INT, 32, "rate", G_TYPE_INT, 
            pipeline_.actualSampleRate(), "channels", G_TYPE_INT, 1, NULL);

    for (src = sources_.begin(); src != sources_.end() and channelIdx != config_.numChannels(); ++src, ++channelIdx)
    {
        GstPad *pad;
        g_object_set(G_OBJECT(*src), "freq", frequencies_[0][channelIdx], "is-live", FALSE, NULL);
        pad = gst_element_get_static_pad(*src, "src");
        g_assert(pad);
        bool capsSet = gst_pad_set_caps(pad, caps);
        g_assert(capsSet);
        g_object_unref(pad);

    }

    gst_caps_unref(caps);
}

const int AudioFileSource::LOOP_INFINITE = -1;

/// Constructor 
AudioFileSource::AudioFileSource(Pipeline &pipeline, const AudioSourceConfig &config) : 
    AudioSource(pipeline, config), BusMsgHandler(&pipeline), aconv_(0), loopCount_(0) 
{
    if (not config_.locationExists())
        THROW_ERROR("File \"" << config_.location() << "\" does not exist");

    aconv_ = AudioSource::pipeline_.makeElement("audioconvert", NULL);

    GstElement * queue = FileSource::acquireAudio(pipeline, config_.location());
    gstlinkable::link(queue, aconv_);
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
    AudioSource(pipeline, config), capsFilter_(0), aconv_(0)
{
    source_ = pipeline_.makeElement(config_.source(), NULL);

    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);

    initCapsFilter(aconv_, capsFilter_);
}

/// Constructor 
AudioPulseSource::AudioPulseSource(const Pipeline &pipeline, const AudioSourceConfig &config) : 
    AudioSource(pipeline, config), 
    capsFilter_(0),
    aconv_(0)
{
    source_ = pipeline_.makeElement(config_.source(), NULL);
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);

    initCapsFilter(aconv_, capsFilter_);
}

/// Constructor 
AudioJackSource::AudioJackSource(const Pipeline &pipeline, const AudioSourceConfig &config) : 
    AudioSource(pipeline, config), 
    capsFilter_(pipeline_.makeElement("capsfilter", 0)), 
    queue_(pipeline_.makeElement("queue", 0)) 
{
    source_ = pipeline_.makeElement(config_.source(), config_.sourceName());

    // use auto-forced connect mode if available
    g_object_set(G_OBJECT(source_), "connect", 2, NULL);

    // setup capsfilter
    GstCaps *caps = 0;
    caps = gst_caps_from_string(getCapsFilterCapsString().c_str());
    g_assert(caps);
    g_object_set(G_OBJECT(capsFilter_), "caps", caps, NULL);

    gst_caps_unref(caps);

    gstlinkable::link(source_, capsFilter_);
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

    gstlinkable::link(capsFilter_, queue_);
}


std::string AudioJackSource::getCapsFilterCapsString()
{
    // force proper number of channels on output
    std::ostringstream capsStr;
    capsStr << "audio/x-raw-float, channels=" << config_.numChannels() 
        << ", rate=" << pipeline_.actualSampleRate();
    LOG_DEBUG("jackAudiosource caps = " << capsStr.str());
    return capsStr.str();
}


bool AudioJackSource::handleMessage(const std::string &path, const std::string &/*arguments*/)
{
    g_assert(source_);
    if (path == "disable-jack-autoconnect")
    {
        g_object_set(G_OBJECT(source_), "connect", 0, NULL);
        return true;
    }
    return false;
}


/// Constructor 
AudioDvSource::AudioDvSource(const Pipeline &pipeline, const AudioSourceConfig &config) : 
    AudioSource(pipeline, config), 
    queue_(0),
    aconv_(0)
{
    queue_ = pipeline_.makeElement("queue", NULL);
    aconv_ = pipeline_.makeElement("audioconvert", NULL);

    // Now the Dv1394 will be able to link this queue to the dvdemux when the audio pad is created
    Dv1394::Instance(pipeline_)->setAudioSink(queue_);
    gstlinkable::link(queue_, aconv_);
}


/// Destructor 
AudioDvSource::~AudioDvSource()
{
    Dv1394::Instance(pipeline_)->unsetAudioSink();
}
