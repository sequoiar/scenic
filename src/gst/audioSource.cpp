// audioSource.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#include <string>
#include <sstream>
#include <cassert>
#include "gstLinkable.h"
#include "logWriter.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "jackUtils.h"
#include "raw1394Util.h"
#include "pipeline.h"
#include "alsa.h"



void AudioSource::init()
{
    init_source();
    sub_init();
    link_elements();
}


void AudioSource::init_source()
{
    sources_.push_back(gst_element_factory_make(config_.source(), NULL));
    assert(sources_[0]);
    aconvs_.push_back(gst_element_factory_make("audioconvert", NULL));
    assert(aconvs_[0]);

    pipeline_.add(sources_);
    pipeline_.add(aconvs_);
}


AudioSource::~AudioSource()
{
    stop();
    pipeline_.remove(aconvs_);
    pipeline_.remove(sources_);
}


void AudioSource::link_elements()
{
    GstLinkable::link(sources_, aconvs_);
}


gboolean AudioSource::base_callback(GstClock *, GstClockTime, GstClockID, gpointer user_data)
{
    return (static_cast<AudioSource*>(user_data)->callback());     // deferred to subclass
}


void InterleavedAudioSource::init()
{
    interleave_.init();
    init_source();
    sub_init();
    link_elements();
}


void InterleavedAudioSource::init_source()
{
    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        sources_.push_back(gst_element_factory_make(config_.source(), NULL));
        assert(sources_[channelIdx]);
        aconvs_.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs_[channelIdx]);
    }

    pipeline_.add(sources_);
    pipeline_.add(aconvs_);
}


void InterleavedAudioSource::link_elements()
{
    GstLinkable::link(sources_, aconvs_);
    GstLinkable::link(aconvs_, interleave_);
}


const double AudioTestSource::FREQUENCY[2][8] = 
{{200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0},
    {300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 1000.0}};


gboolean AudioTestSource::callback()
{
    toggle_frequency();
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
    GstIter src;

    const double GAIN = 1.0 / config_.numChannels();        // so sum of tones' amplitude equals 1.0
    int channelIdx = 0;

    GstCaps *caps = gst_caps_new_simple("audio/x-raw-int", "endianness", G_TYPE_INT, 1234, "signed", 
            G_TYPE_BOOLEAN, TRUE, "width", G_TYPE_INT, 32, "depth", G_TYPE_INT, 32, "rate", G_TYPE_INT, 
            GstBase::SAMPLE_RATE, "channels", G_TYPE_INT, 1, NULL);

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

    clockId_ = pipeline_.add_clock_callback(base_callback, this);
}


AudioTestSource::~AudioTestSource()
{
    stop();
    pipeline_.remove_clock_callback(clockId_);
}


AudioFileSource::AudioFileSource(const AudioConfig &config) : AudioSource(config), decoders_(), loopCount_(0) 
{
    loopCount_ = config_.loop();
}

void AudioFileSource::sub_init()
{
    assert(config_.fileExists());

    g_object_set(G_OBJECT(sources_[0]), "location", config_.location(), NULL);

    decoders_.push_back(gst_element_factory_make("decodebin", NULL));
    assert(decoders_[0]);
    pipeline_.add(decoders_);

    GstIter dec = decoders_.begin();
    GstIter aconv = aconvs_.begin();
    for (; aconv != aconvs_.end(), dec != decoders_.end(); ++dec, ++aconv)
    {
        g_signal_connect(*dec, "new-decoded-pad",
                G_CALLBACK(AudioFileSource::cb_new_src_pad),
                static_cast<void *>(*aconv));
    }
    // register this filesrc to handle EOS msg and loop if specified
    pipeline_.subscribe(this);
}



bool AudioFileSource::handleBusMsg(_GstMessage *msg)
{
    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
    {
        LOG_DEBUG("Got end of stream, here's where we should playback if needed");
        if (loopCount_ != 0)
        {
            LOG_DEBUG("playback about to restart, " << loopCount_ << " times to go");
            restartPlayback();
        }
        return true;
    }
    return false;
}


void AudioFileSource::restartPlayback()
{
    const gint64 BEGIN_TIME_NS = 0;
    pipeline_.seekTo(BEGIN_TIME_NS);
    if (loopCount_ > 0)  // avoids endless decrements
        loopCount_--;
}

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

    assert(GstLinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


void AudioFileSource::link_elements()
{
    GstLinkable::link(sources_, decoders_);
}


AudioFileSource::~AudioFileSource()
{
    stop();
    pipeline_.remove(decoders_);
}

void AudioAlsaSource::sub_init()
{
    if (Jack::is_running())
        THROW_ERROR("Jack is running, ALSA unavailable");
    g_object_set(G_OBJECT(sources_[0]), "device", Alsa::DEVICE_NAME, NULL);
    
    // otherwise alsasrc defaults to 2 channels when using plughw
    std::ostringstream capsStr;
    capsStr << "audio/x-raw-int, channels=" << config_.numChannels();
         
    GstCaps *alsaCaps = gst_caps_from_string(capsStr.str().c_str());
    capsFilter_ = gst_element_factory_make("capsfilter", NULL);
    assert(capsFilter_);
    g_object_set(G_OBJECT(capsFilter_), "caps", alsaCaps, NULL);
    pipeline_.add(capsFilter_);
    gst_caps_unref(alsaCaps);
}


void AudioAlsaSource::link_elements()
{
    GstLinkable::link(sources_, aconvs_);
    GstLinkable::link(aconvs_[0], capsFilter_);
}


void AudioJackSource::sub_init()
{
    if (!Jack::is_running())
        THROW_ERROR("Jack is not running");

#if 0
    // turn off autoconnect to avoid Jack-killing input-output feedback loop, i.e.
    // jackOut -> jackIn -> jackOut ->jackIn.....
    for (GstIter src = sources_.begin(); src != sources_.end(); ++src)
        g_object_set(G_OBJECT(*src), "connect", 0, NULL);
#endif

    if (GstBase::SAMPLE_RATE != Jack::samplerate())
        THROW_CRITICAL("Jack's sample rate of " << Jack::samplerate()
                << " does not match default sample rate " << GstBase::SAMPLE_RATE);
}


AudioDvSource::~AudioDvSource()
{
    stop();
    if (pipeline_.findElement(config_.source()) != NULL)
        pipeline_.remove(sources_);
    sources_[0] = NULL;
}

void AudioDvSource::init_source()
{
    if (!Raw1394::cameraIsReady())
        THROW_ERROR("Camera is not ready.");

    sources_.push_back(pipeline_.findElement(config_.source()));  // see if it already exists from VideoDvSource
    dvIsNew_ = sources_[0] == NULL;

    if (dvIsNew_)
    {
        sources_[0] = gst_element_factory_make(config_.source(), config_.source());
        assert(sources_[0]);
        pipeline_.add(sources_);
    }
    aconvs_.push_back(gst_element_factory_make("audioconvert", NULL));
    assert(aconvs_[0]);

    pipeline_.add(aconvs_);
}


void AudioDvSource::sub_init()
{
    demux_ = pipeline_.findElement("dvdemux");
    dvIsNew_ = demux_ == NULL;
    if (dvIsNew_)
        assert(demux_ = gst_element_factory_make("dvdemux", "dvdemux"));
    else
        assert(demux_);
    assert(queue_ = gst_element_factory_make("queue", NULL));

    // demux has dynamic pads
    if (dvIsNew_)
        pipeline_.add(demux_);
    pipeline_.add(queue_);

    // demux srcpad must be linked to queue sink pad at runtime
    g_signal_connect(demux_, "pad-added",
            G_CALLBACK(AudioDvSource::cb_new_src_pad),
            static_cast<void *>(queue_));
}


void AudioDvSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, void *data)
{
    if (std::string("video") == gst_pad_get_name(srcPad))
    {
        LOG_DEBUG("Ignoring video stream from DV");
        return;
    }
    else if (std::string("audio") == gst_pad_get_name(srcPad))
    {
        LOG_DEBUG("Got audio stream from DV");
    }
    else{
        LOG_DEBUG("Ignoring unknown stream from DV");
        return;
    }
    GstElement *sinkElement = static_cast<GstElement *>(data);
    GstPad *sinkPad;

    sinkPad = gst_element_get_static_pad(sinkElement, "sink");

    if (GST_PAD_IS_LINKED(sinkPad))
    {
        g_object_unref(sinkPad);        // don't link more than once
        return;
    }
    LOG_DEBUG("AudioDvSource: linking new srcpad to sinkpad.");
    assert(GstLinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


void AudioDvSource::link_elements()
{
    if (dvIsNew_)
        GstLinkable::link(sources_[0], demux_);
    GstLinkable::link(queue_, aconvs_[0]);
}


