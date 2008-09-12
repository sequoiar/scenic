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
#include <cassert>
#include "gstLinkable.h"
#include "logWriter.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "jackUtils.h"
#include "pipeline.h"


bool AudioSource::init()
{
    init_source();
    sub_init();
    link_elements();
    return true;
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
    assert(stop());
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


bool InterleavedAudioSource::init()
{
    interleave_.init();
    init_source();
    sub_init();
    link_elements();
    return true;
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


gboolean AudioTestSource::callback()
{
    toggle_frequency();
    return TRUE;
}


void AudioTestSource::toggle_frequency()
{
    static const double FREQUENCY[2][8] =
    {{200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0},
     {300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 1000.0}};

    int i = 0;

    for (GstIter iter = sources_.begin(); iter != sources_.end(); ++iter)
        g_object_set(G_OBJECT(*iter), "freq", FREQUENCY[offset_][i++], NULL);

    offset_ = (offset_ == 0) ? 1 : 0;
}


void AudioTestSource::sub_init()
{
    GstIter src;

    const double GAIN = 1.0 / config_.numChannels();        // so sum of tones' amplitude equals 1.0
    double frequency = 100.0;

    // is-live must be true for clocked callback to work properly
    for (src = sources_.begin(); src != sources_.end(); ++src, frequency += 100.0)
        g_object_set(G_OBJECT(*src), "volume", GAIN, "freq", frequency, "is-live", TRUE, NULL);

    clockId_ = pipeline_.add_clock_callback(base_callback, this);
}


AudioTestSource::~AudioTestSource()
{
    assert(stop());
    pipeline_.remove_clock_callback(clockId_);
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
}


void AudioFileSource::cb_new_src_pad(GstElement *  /*srcElement*/, GstPad * srcPad, gboolean /*last*/,
                                     gpointer data)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG("Pad is already linked.", DEBUG);
        return;
    }
    else if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
    {
        LOG("Pad is not a source", DEBUG);
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
    assert(stop());
    pipeline_.remove(decoders_);
}


void AudioAlsaSource::sub_init()
{
    if (Jack::is_running())
        LOG("Jack is running, ALSA unavailable", ERROR);
}


void AudioJackSource::sub_init()
{
    if (!Jack::is_running())
        LOG("Jack is not running", ERROR);

    // turn off autoconnect to avoid Jack-killing input-output feedback loop, i.e.
    // jackOut -> jackIn -> jackOut ->jackIn.....
    for (GstIter src = sources_.begin(); src != sources_.end(); ++src)
        g_object_set(G_OBJECT(*src), "connect", 0, NULL);
}


AudioDvSource::~AudioDvSource()
{
    assert(stop());
    if (pipeline_.findElement(config_.source()) != NULL)
        pipeline_.remove(sources_);
    sources_[0] = NULL;
}

void AudioDvSource::init_source()
{
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
        LOG("Ignoring video stream from DV", DEBUG);
        return;
    }
    else if (std::string("audio") == gst_pad_get_name(srcPad))
    {
        LOG("Got audio stream from DV", DEBUG);
    }
    else{
        LOG("Ignoring unknown stream from DV", DEBUG);
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
    LOG("AudioDvSource: linking new srcpad to sinkpad.", DEBUG);
    assert(GstLinkable::link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


void AudioDvSource::link_elements()
{
    if (dvIsNew_)
        GstLinkable::link(sources_[0], demux_);
    GstLinkable::link(queue_, aconvs_[0]);
}


