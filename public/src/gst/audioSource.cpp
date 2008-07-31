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

#include <iostream>
#include <string>
#include <cassert>
#include "logWriter.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "jackUtils.h"

AudioSource::AudioSource(const AudioConfig &config)
    : config_(config), interleave_(config), sources_(0), aconvs_(0)
{}

void AudioSource::init()
{
    init_source();
    sub_init();
    link_elements();
    link_interleave();
}


void AudioSource::init_source()
{
    interleave_.init();

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        sources_.push_back(gst_element_factory_make(config_.source(), NULL));
        assert(sources_[channelIdx]);
        aconvs_.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs_[channelIdx]);
    }

    pipeline_.add_vector(sources_);
    pipeline_.add_vector(aconvs_);
}


AudioSource::~AudioSource()
{
    assert(stop());
    pipeline_.remove_vector(aconvs_);
    pipeline_.remove_vector(sources_);
}


void AudioSource::link_elements()
{
    link_element_vectors(sources_, aconvs_);
}


void AudioSource::link_interleave()
{
    interleave_.link_to_sources(aconvs_);
}


void AudioSource::link_to_sink(GstElement *sink)
{
    interleave_.link_to_sink(sink);
}


gboolean AudioSource::base_callback(GstClock *clock, GstClockTime time, GstClockID id,
                                    gpointer user_data)
{
    return (static_cast<AudioSource*>(user_data)->callback());     // deferred to subclass
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


void AudioFileSource::init_source()
{
    // just make one multichannel filesrc
    sources_.push_back(gst_element_factory_make(config_.source(), NULL));
    assert(sources_[0]);
    g_object_set(G_OBJECT(sources_[0]), "location", "test_signal8.wav", NULL);

    aconvs_.push_back(gst_element_factory_make("audioconvert", NULL));
    assert(aconvs_[0]);

    pipeline_.add_vector(sources_);
    pipeline_.add_vector(aconvs_);
}


void AudioFileSource::sub_init()
{
    decoders_.push_back(gst_element_factory_make("decodebin", NULL));
    assert(decoders_[0]);
    pipeline_.add_vector(decoders_);

    GstIter dec = decoders_.begin();
    GstIter aconv = aconvs_.begin();
    for (; aconv != aconvs_.end(), dec != decoders_.end(); ++dec, ++aconv)
    {
        g_signal_connect(*dec, "new-decoded-pad",
                         G_CALLBACK(AudioFileSource::cb_new_src_pad),
                         static_cast<void *>(*aconv));
    }
}


void AudioFileSource::cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, gboolean last,
                                     gpointer data)
{
    if (gst_pad_is_linked(srcPad))
    {
        LOG("Pad is already linked.")
        return;
    }
    else if (gst_pad_get_direction(srcPad) != GST_PAD_SRC)
    {
        LOG("Pad is not a source");
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

    assert(link_pads(srcPad, sinkPad));
    gst_object_unref(sinkPad);
}


void AudioFileSource::link_elements()
{
    link_element_vectors(sources_, decoders_);
}


void AudioFileSource::link_to_sink(GstElement *sink)
{
    assert(gst_element_link(aconvs_[0], sink));
}


AudioFileSource::~AudioFileSource()
{
    assert(stop());
    pipeline_.remove_vector(decoders_);
}


void AudioJackSource::sub_init()
{
    assert(jack_is_running());

    // turn off autoconnect to avoid Jack-killing input-output feedback loop, i.e.
    // jackOut -> jackIn -> jackOut ->jackIn.....
    for (GstIter src = sources_.begin(); src != sources_.end(); ++src)
        g_object_set(G_OBJECT(*src), "connect", 0, NULL);
}


