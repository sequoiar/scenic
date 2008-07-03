
// audioSender.cpp
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
//
// TODO: Client code should just call this with some kind of parameter object which
// specifies number of channels, how to compress it (if at all), and host and port info.

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <vector>

#include <gst/gst.h>
#include <gst/audio/multichannel.h>

#include "lo/lo.h"

#include "logWriter.h"
#include "audioSender.h"

AudioSender::AudioSender(const AudioConfig & config)
	: MediaBase(dynamic_cast < const MediaConfig & >(config)),config_(config), encoder_(0), payloader_(0), interleave_(0), sink_(0)
{
	// empty
}

AudioSender::~AudioSender()
{
	assert(stop());

	pipeline_.remove_vector(queues_);
	pipeline_.remove_vector(aconvs_);
	pipeline_.remove_vector(decoders_);
	pipeline_.remove_vector(sources_);

	pipeline_.remove(sink_);
	pipeline_.remove(payloader_);
	pipeline_.remove(encoder_);
	pipeline_.remove(interleave_);
}

void AudioSender::init_interleave()
{
	interleave_ = gst_element_factory_make("interleave", NULL);
	assert(interleave_);

	pipeline_.add(interleave_);

	set_channel_layout();
}

void AudioSender::init_source()
{
	init_interleave();

	GstIter src, aconv, queue;

	// common to all
	for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
	{
        sources_.push_back(gst_element_factory_make(config_.source(), NULL));
        assert(sources_[channelIdx]);
        aconvs_.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs_[channelIdx]);
        queues_.push_back(gst_element_factory_make("queue", NULL));
        assert(queues_[channelIdx]);

    }

    pipeline_.add_vector(sources_);
    pipeline_.add_vector(aconvs_);
    pipeline_.add_vector(queues_);

    // FIXME: replace with subclasses?
    if (config_.hasTestSrc()) {
        const double GAIN = 1.0 / config_.numChannels();        // so sum of tones equals 1.0
        double frequency = 100.0;

        for (src = sources_.begin(); src != sources_.end(); ++src, frequency += 100.0)
            g_object_set(G_OBJECT(*src), "volume", GAIN, "freq", frequency, "is-live", TRUE, NULL);

        for (src = sources_.begin(), aconv = aconvs_.begin(); src != sources_.end(); ++src, ++aconv)
            assert(gst_element_link_many(*src, *aconv, interleave_, NULL));
    }
    else if (config_.hasFileSrc()) {    // adds decoder, which has a dynamic pad
        GstIter dec;

        for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
        {
            decoders_.push_back(gst_element_factory_make("wavparse", NULL));
            assert(decoders_[channelIdx]);
        }

        pipeline_.add_vector(decoders_);

        // FIXME: location should be changeable
        for (src = sources_.begin(); src != sources_.end(); ++src)
            g_object_set(G_OBJECT(*src), "location", "audiofile.pcm", NULL);

        for (dec = decoders_.begin(), aconv = aconvs_.begin(); dec != decoders_.end(); ++dec, ++aconv)
        {
            g_signal_connect(*dec, "pad-added", G_CALLBACK(Pipeline::cb_new_src_pad), (void *) *aconv);
        }

        for (src = sources_.begin(), dec = decoders_.begin(); src != sources_.end(); ++src, ++dec)
            assert(gst_element_link(*src, *dec));

        for (aconv = aconvs_.begin(), aconv = aconvs_.begin(); aconv != aconvs_.end(); ++aconv)
            assert(gst_element_link(*aconv, interleave_));
    }
    else if (config_.hasAlsaSrc()) {
        // initialize alsa source(s)
    }
    else if (config_.hasJackSrc()) {
        // initialize jack source(s)
    }
}

void AudioSender::init_codec()
{
    if (!config_.hasCodec()) {
        encoder_ = gst_element_factory_make(config_.codec(), NULL);
        assert(encoder_);
    }
}

void AudioSender::init_sink()
{
    if (config_.isNetworked()) {
        payloader_ = gst_element_factory_make("rtpvorbispay", NULL);
        assert(payloader_);

        pipeline_.add(encoder_);
        pipeline_.add(payloader_);

        assert(gst_element_link_many(interleave_, encoder_, payloader_, NULL));

        session_.add(payloader_, dynamic_cast < const MediaConfig & >(config_));
    }
    else                        // local version
    {
        sink_ = gst_element_factory_make("jackaudiosink", NULL);
        assert(sink_);
        g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

        pipeline_.add(sink_);

        assert(gst_element_link_many(interleave_, sink_, NULL));
    }
}

// returns caps for last sink, needs to be sent to receiver for rtpvorbisdepay

void AudioSender::send_caps() const
{
    LOG("Sending caps...");

    lo_address t = lo_address_new(NULL, "7770");
    if (lo_send(t, "/audio/rx/caps", "s", session_.caps_str()) == -1)
        std::cerr << "OSC error " << lo_address_errno(t) << ": " << lo_address_errstr(t) << std::endl;
}

// set channel layout for interleave element
void AudioSender::set_channel_layout()
{
    GValue val = { 0, };
    GValueArray *arr;           // for channel position layout
    arr = g_value_array_new(config_.numChannels());

    g_object_set(interleave_, "channel-positions-from-input", FALSE, NULL);

    g_value_init(&val, GST_TYPE_AUDIO_CHANNEL_POSITION);

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        g_value_set_enum(&val, VORBIS_CHANNEL_POSITIONS[config_.numChannels() - 1][channelIdx]);
        g_value_array_append(arr, &val);
        g_value_reset(&val);
    }
    g_value_unset(&val);

    g_object_set(interleave_, "channel-positions", arr, NULL);
    g_value_array_free(arr);
}

bool AudioSender::start()
{
    MediaBase::start();

    if (config_.isNetworked()) {
        std::cout << "Sending audio to host " << config_.remoteHost() << " on port " << config_.port()
            << std::endl;

        wait_until_playing();
        send_caps();
    }

    return true;
}

// courtesy of vorbisenc.c

const GstAudioChannelPosition AudioSender::VORBIS_CHANNEL_POSITIONS[][8] = {
    {                           /* Mono */
        GST_AUDIO_CHANNEL_POSITION_FRONT_MONO
    },
    {                           /* Stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                           /* Stereo + Centre */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                           /* Quadraphonic */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
    },
    {                           /* Stereo + Centre + rear stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
    },
    {                           /* Full 5.1 Surround */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_LFE,
    },
    {                           /* Not defined by spec, GStreamer default */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE,
        GST_AUDIO_CHANNEL_POSITION_REAR_CENTER,
    },
    {                           /* Not defined by spec, GStreamer default */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE,
        GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT,
        GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT,
    },
};

