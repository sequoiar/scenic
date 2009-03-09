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
    // otherwise alsasrc defaults to 2 channels when using plughw
    std::ostringstream capsStr;
    capsStr << "audio/x-raw-int, channels=" << config_.numChannels() 
        << ", clock-rate=" << Pipeline::SAMPLE_RATE;
    std::cout << "Audiosource caps = " << capsStr.str();
    return capsStr.str();
}


void AudioSource::setupCapsFilter(GstElement* &aconv, GstElement* &capsFilter)
{
    // setup capsfilter
    GstCaps *caps = 0;
    caps = gst_caps_from_string(getCapsFilterCapsString().c_str());
    assert(caps);
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
{}


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
            Pipeline::SAMPLE_RATE, "channels", G_TYPE_INT, 1, NULL);

    // is-live must be true for clocked callback to work properly
    for (src = sources_.begin(); src != sources_.end() && channelIdx != config_.numChannels(); ++src, ++channelIdx)
    {
        GstPad *pad;
        g_object_set(G_OBJECT(*src), "volume", GAIN, "freq", FREQUENCY[0][channelIdx], "is-live", TRUE, NULL);
        assert(pad = gst_element_get_static_pad(*src, "src"));
        assert(gst_pad_set_caps(pad, caps));
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
AudioFileSource::AudioFileSource(const AudioSourceConfig &config) : AudioSource(config), decoder_(0), aconv_(0), loopCount_(0) 
{}

void AudioFileSource::loop(int nTimes)
{
    if (nTimes < -1)
        THROW_ERROR("Loop setting must be either >= 0 , or -1 for infinite looping");

    loopCount_ = nTimes;
}

void AudioFileSource::sub_init()
{
    AudioSource::sub_init();

    assert(config_.fileExists());

    g_object_set(G_OBJECT(source_), "location", config_.location(), NULL);

    decoder_ = Pipeline::Instance()->makeElement("decodebin", NULL);
    decoder_ = Pipeline::Instance()->makeElement("aconv", NULL);

    g_signal_connect(decoder_, "new-decoded-pad",
            G_CALLBACK(AudioFileSource::cb_new_src_pad),
            static_cast<void *>(aconv_));
    // register this filesrc to handle EOS msg and loop if specified
    Pipeline::Instance()->subscribe(this);
    
    gstlinkable::link(source_, decoder_);
}


/// Handles EOS signal from bus, which may mean repeating playback of the file 
bool AudioFileSource::handleBusMsg(_GstMessage *msg)
{
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
    {
        LOG_DEBUG("Got end of stream, here's where we should playback if needed");
        if (loopCount_ > 0 || loopCount_ == AudioFileSource::LOOP_INFINITE)
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


/// Handles decoders' dynamically created pad(s) by linking them to the rest of the pipeline 
void AudioFileSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, gboolean /*last*/,
        gpointer data)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG_DEBUG("Pad is already linked.");
        return;
    }
    else if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
    {
        LOG_DEBUG("Pad is not a source");
        return;
    }
    GstElement *sinkElement = static_cast<GstElement*>(data);
    GstStructure *str;
    GstPad *sinkPad;
    GstCaps *caps;

    sinkPad = gst_element_get_static_pad(sinkElement, "sink");
    if (GST_PAD_IS_LINKED(sinkPad))
    {
        g_object_unref(sinkPad);
        return;
    }
    /* check media type */
    caps = gst_pad_get_caps(srcPad);
    str = gst_caps_get_structure(caps, 0);
    if (!g_strrstr(gst_structure_get_name(str), "audio")) {
        gst_caps_unref(caps);
        gst_object_unref(srcPad);
        return;
    }
    gst_caps_unref(caps);

    assert(gstlinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


/// Destructor 
AudioFileSource::~AudioFileSource()
{
    Pipeline::Instance()->remove(&decoder_);
}


/// Constructor 
AudioAlsaSource::AudioAlsaSource(const AudioSourceConfig &config, unsigned long long bufferTime) : 
    AudioSource(config), capsFilter_(0), aconv_(0), bufferTime_(bufferTime)
{}

/// Destructor 
AudioAlsaSource::~AudioAlsaSource()
{
    Pipeline::Instance()->remove(&aconv_);
    Pipeline::Instance()->remove(&capsFilter_);
}

// FIXME : encapsulate capsfilter stuff into one class

void AudioAlsaSource::sub_init()
{
    AudioSource::sub_init();

    if (Jack::is_running())
        THROW_ERROR("Jack is running, ALSA unavailable");
    g_object_set(G_OBJECT(source_), "device", alsa::DEVICE_NAME, NULL);
    g_object_set(G_OBJECT(source_), "buffer-time", bufferTime_, NULL);

    setupCapsFilter(aconv_, capsFilter_);
}


/// Constructor 
AudioPulseSource::AudioPulseSource(const AudioSourceConfig &config, unsigned long long bufferTime) : 
    AudioSource(config), 
    capsFilter_(0),
    aconv_(0),
    bufferTime_(bufferTime)
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

    g_object_set(G_OBJECT(source_), "buffer-time", bufferTime_, NULL);
    setupCapsFilter(aconv_, capsFilter_);
}



/// Constructor 
AudioJackSource::AudioJackSource(const AudioSourceConfig &config, unsigned long long bufferTime) : 
    AudioSource(config), capsFilter_(0), aconv_(0), bufferTime_(bufferTime)
{}


/// Destructor 
AudioJackSource::~AudioJackSource()
{
    Pipeline::Instance()->remove(&aconv_);
    Pipeline::Instance()->remove(&capsFilter_);
}


void AudioJackSource::sub_init()
{
    source_ = Pipeline::Instance()->makeElement("jackaudiosrc", NULL);  // because of fastjackaudiosrc

    if (!Jack::is_running())
        THROW_ERROR("Jack is not running");

#if 1
    // uncomment to turn off autoconnect to avoid Jack-killing input-output feedback loop, i.e.
    // jackOut -> jackIn -> jackOut ->jackIn.....
    g_object_set(G_OBJECT(source_), "connect", 0, NULL);
#endif
    // /TODO: fine tune this in conjunction with jitterbuffer
    
    g_object_set(G_OBJECT(source_), "buffer-time", bufferTime_, NULL);

    setupCapsFilter(aconv_, capsFilter_);

    if (Pipeline::SAMPLE_RATE != Jack::samplerate())
        THROW_CRITICAL("Jack's sample rate of " << Jack::samplerate()
                << " does not match default sample rate " << Pipeline::SAMPLE_RATE);
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

