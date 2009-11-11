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

#include "util.h"

#include "gstLinkable.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "jackUtils.h"
#include "pipeline.h"
#include "alsa.h"
#include "dv1394.h"
#include "fileSource.h"

#include <iostream>

/// Constructor 
AudioSource::AudioSource(const AudioSourceConfig &config) : 
    config_(config), 
    source_(0)
{}

void AudioSource::init()
{
    sub_init();
}

/// Initialize source_
void AudioSource::sub_init()
{
    source_ = Pipeline::Instance()->makeElement(config_.source(), NULL);
}


/// Destructor 
AudioSource::~AudioSource()
{
    Pipeline::Instance()->remove(&source_);
}


std::string AudioSource::getCapsFilterCapsString()
{
    // force proper number of channels on output
    std::ostringstream capsStr;
    capsStr << "audio/x-raw-int, channels=" << config_.numChannels() 
        << ", rate=" << Pipeline::Instance()->actualSampleRate();
    LOG_DEBUG("Audiosource caps = " << capsStr.str());
    return capsStr.str();
}


void AudioSource::initCapsFilter(GstElement* &aconv, GstElement* &capsFilter)
{
    // setup capsfilter
    GstCaps *caps = 0;
    caps = gst_caps_from_string(getCapsFilterCapsString().c_str());
    tassert(caps);
    capsFilter = Pipeline::Instance()->makeElement("capsfilter", NULL);
    aconv = Pipeline::Instance()->makeElement("audioconvert", NULL);
    g_object_set(G_OBJECT(capsFilter), "caps", caps, NULL);

    gst_caps_unref(caps);
    
    gstlinkable::link(source_, aconv);
    gstlinkable::link(aconv, capsFilter);
}


/// Constructor 
InterleavedAudioSource::InterleavedAudioSource(const AudioSourceConfig &config) : 
    AudioSource(config), interleave_(config_), sources_(), aconvs_()
{}

/// Destructor 
InterleavedAudioSource::~InterleavedAudioSource() 
{
    Pipeline::Instance()->remove(aconvs_);
    Pipeline::Instance()->remove(sources_);
}


/// Overridden source initializer, which must initialize this object's Interleave object 
void InterleavedAudioSource::sub_init()
{
    interleave_.init();

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        sources_.push_back(Pipeline::Instance()->makeElement(config_.source(), NULL));
        aconvs_.push_back(Pipeline::Instance()->makeElement("audioconvert", NULL));
    }
    gstlinkable::link(sources_, aconvs_);
    gstlinkable::link(aconvs_, interleave_);
}


const double AudioTestSource::FREQUENCY[2][8] = 
{{200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0},
    {300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 1000.0}};

/// Constructor 
AudioTestSource::AudioTestSource(const AudioSourceConfig &config) : 
    InterleavedAudioSource(config), 
    clockId_(0), 
    offset_(0) 
{}


/// Asynchronous timed callback which will periodically toggle the frequency output by each channel 
gboolean AudioTestSource::timedCallback(GstClock *, GstClockTime, GstClockID, gpointer user_data)
{
    AudioTestSource * context = static_cast<AudioTestSource*>(user_data);
    context->toggle_frequency();
    return TRUE;
}


void AudioTestSource::toggle_frequency()
{
    int i = 0;

    for (GstIter iter = sources_.begin(); iter != sources_.end(); ++iter)
        g_object_set(G_OBJECT(*iter), "freq", FREQUENCY[offset_][i++], NULL);

    offset_ = (offset_ == 0) ? 1 : 0;
}


void AudioTestSource::sub_init()
{
    InterleavedAudioSource::sub_init();

    GstIter src;

    const double GAIN = 1.0 / config_.numChannels();        // so sum of tones' amplitude equals 1.0
    int channelIdx = 0;

    GstCaps *caps = gst_caps_new_simple("audio/x-raw-int", "endianness", G_TYPE_INT, 1234, "signed", 
            G_TYPE_BOOLEAN, TRUE, "width", G_TYPE_INT, 32, "depth", G_TYPE_INT, 32, "rate", G_TYPE_INT, 
            Pipeline::Instance()->actualSampleRate(), "channels", G_TYPE_INT, 1, NULL);

    for (src = sources_.begin(); src != sources_.end() and channelIdx != config_.numChannels(); ++src, ++channelIdx)
    {
        GstPad *pad;
        g_object_set(G_OBJECT(*src), "volume", GAIN, "freq", FREQUENCY[0][channelIdx], "is-live", TRUE, NULL);
        tassert(pad = gst_element_get_static_pad(*src, "src"));
        tassert(gst_pad_set_caps(pad, caps));
        g_object_unref(pad);

    }

    gst_caps_unref(caps);

    clockId_ = Pipeline::Instance()->add_clock_callback(timedCallback, this);
}

/// Destructor 
AudioTestSource::~AudioTestSource()
{
    Pipeline::Instance()->remove_clock_callback(clockId_);
}


const int AudioFileSource::LOOP_INFINITE = -1;

/// Constructor 
AudioFileSource::AudioFileSource(const AudioSourceConfig &config) : AudioSource(config), aconv_(0), loopCount_(0) 
{}

void AudioFileSource::loop(int nTimes)
{
    if (nTimes < -1)
        THROW_ERROR("Loop setting must be either >= 0 , or -1 for infinite looping");

    loopCount_ = nTimes;
}

void AudioFileSource::sub_init()
{
    tassert(config_.locationExists());

    aconv_ = Pipeline::Instance()->makeElement("audioconvert", NULL);
    
    GstElement * queue = FileSource::acquireAudio(config_.location());
    gstlinkable::link(queue, aconv_);
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

        return true;
    }
    return false;
}


void AudioFileSource::restartPlayback()
{
    const gint64 BEGIN_TIME_NS = 0;
    Pipeline::Instance()->seekTo(BEGIN_TIME_NS);
    if (loopCount_ > 0)  // avoids endless decrements
        loopCount_--;
}


/// Destructor 
AudioFileSource::~AudioFileSource()
{
    Pipeline::Instance()->remove(&aconv_);
    FileSource::releaseAudio(config_.location());
}


/// Constructor 
AudioAlsaSource::AudioAlsaSource(const AudioSourceConfig &config) : 
    AudioSource(config), capsFilter_(0), aconv_(0)
{}

/// Destructor 
AudioAlsaSource::~AudioAlsaSource()
{
    Pipeline::Instance()->remove(&aconv_);
    Pipeline::Instance()->remove(&capsFilter_);
}

void AudioAlsaSource::sub_init()
{
    AudioSource::sub_init();

    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);
    else
        g_object_set(G_OBJECT(source_), "device", alsa::DEVICE_NAME, NULL);

    initCapsFilter(aconv_, capsFilter_);
}


/// Constructor 
AudioPulseSource::AudioPulseSource(const AudioSourceConfig &config) : 
    AudioSource(config), 
    capsFilter_(0),
    aconv_(0)
{}


/// Destructor 
AudioPulseSource::~AudioPulseSource()
{
    Pipeline::Instance()->remove(&aconv_);
    Pipeline::Instance()->remove(&capsFilter_);
}


void AudioPulseSource::sub_init()
{
    AudioSource::sub_init();
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(source_), "device", config_.deviceName(), NULL);
    else
        g_object_set(G_OBJECT(source_), "device", alsa::DEVICE_NAME, NULL);

    initCapsFilter(aconv_, capsFilter_);
}



/// Constructor 
AudioJackSource::AudioJackSource(const AudioSourceConfig &config) : 
    AudioSource(config), capsFilter_(0)
{
}


/// Destructor 
AudioJackSource::~AudioJackSource()
{
    Pipeline::Instance()->remove(&capsFilter_);
}

std::string AudioJackSource::getCapsFilterCapsString()
{
    // force proper number of channels on output
    std::ostringstream capsStr;
    capsStr << "audio/x-raw-float, channels=" << config_.numChannels() 
        << ", rate=" << Pipeline::Instance()->actualSampleRate();
    LOG_DEBUG("jackAudiosource caps = " << capsStr.str());
    return capsStr.str();
}

void AudioJackSource::sub_init()
{
    source_ = Pipeline::Instance()->makeElement(config_.source(), config_.sourceName());

    // use auto-forced connect mode if available
    if (Jack::autoForcedSupported(source_))
        g_object_set(G_OBJECT(source_), "connect", 2, NULL);
    
    // setup capsfilter
    GstCaps *caps = 0;
    caps = gst_caps_from_string(getCapsFilterCapsString().c_str());
    tassert(caps);
    capsFilter_ = Pipeline::Instance()->makeElement("capsfilter", NULL);
    g_object_set(G_OBJECT(capsFilter_), "caps", caps, NULL);

    gst_caps_unref(caps);
    
    gstlinkable::link(source_, capsFilter_);
}


bool AudioJackSource::handleMessage(const std::string &path, const std::string &/*arguments*/)
{
    assert(source_);
    if (path == "disable-jack-autoconnect")
    {
        g_object_set(G_OBJECT(source_), "connect", 0, NULL);
        return true;
    }
    return false;
}


/// Constructor 
AudioDvSource::AudioDvSource(const AudioSourceConfig &config) : 
    AudioSource(config), 
    queue_(0),
    aconv_(0)
{}


/// Destructor 
AudioDvSource::~AudioDvSource()
{
    Pipeline::Instance()->remove(&queue_);
    Dv1394::Instance()->unsetAudioSink();
}


void AudioDvSource::sub_init()
{
    queue_ = Pipeline::Instance()->makeElement("queue", NULL);
    aconv_ = Pipeline::Instance()->makeElement("audioconvert", NULL);

    // Now the Dv1394 will be able to link this queue to the dvdemux when the audio pad is created
    Dv1394::Instance()->setAudioSink(queue_);
    gstlinkable::link(queue_, aconv_);
}

