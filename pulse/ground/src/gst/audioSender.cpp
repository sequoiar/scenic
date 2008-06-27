
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

AudioSender::AudioSender(const AudioConfig& config) : config_(config)
{
    // empty
}



AudioSender::~AudioSender() 
{
    // empty
}


void AudioSender::init_interleave()
{
    interleave_ = gst_element_factory_make("interleave", NULL);
    assert(interleave_);

    gst_bin_add(GST_BIN(pipeline_), interleave_);

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
        
        gst_bin_add_many(GST_BIN(pipeline_), sources_[channelIdx], aconvs_[channelIdx], 
                queues_[channelIdx], NULL);
    }

    // FIXME: replace with subclasses?
    if (config_.hasTestSrc())
    {
        const double GAIN = 1.0 / config_.numChannels(); // so sum of tones equals 1.0
        double frequency = 100.0;

        for (src = sources_.begin(); src != sources_.end(); ++src, frequency += 100.0)
            g_object_set(G_OBJECT(*src), "volume", GAIN, "freq", frequency, "is-live", TRUE, NULL);
        
        for (src = sources_.begin(), aconv = aconvs_.begin(); src != sources_.end(); ++src, ++aconv)
            assert(gst_element_link_many(*src, *aconv, interleave_, NULL));
    }
    else if (config_.hasFileSrc())      // adds decoder, which has a dynamic pad
    {
        GstIter dec;

        for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
        {
            decoders_.push_back(gst_element_factory_make("wavparse", NULL));
            assert(decoders_[channelIdx]);
        }

        // FIXME: location should be changeable
        for (src = sources_.begin(); src != sources_.end(); ++src) 
            g_object_set(G_OBJECT(*src), "location", "audiofile.pcm", NULL);
        
        for (dec = decoders_.begin(), aconv = aconvs_.begin(); dec != decoders_.end(); ++dec, ++aconv)
        {
            gst_bin_add(GST_BIN(pipeline_), *dec);
            g_signal_connect(*dec, "pad-added", G_CALLBACK(cb_new_src_pad), (void *) *aconv);
        }
        
        for (src = sources_.begin(), dec = decoders_.begin(); src != sources_.end(); ++src, ++dec)
            assert(gst_element_link(*src, *dec));
        
        for (aconv = aconvs_.begin(), aconv = aconvs_.begin(); aconv != aconvs_.end(); ++aconv)
            assert(gst_element_link(*aconv, interleave_));
    }
    else if (config_.hasAlsaSrc())
    {
        // initialize alsa source(s) 
    }
    else if (config_.hasJackSrc())
    {
        // initialize jack source(s) 
    }
}



void AudioSender::init_codec()
{
    if (!config_.hasCodec())
    {
        encoder_ = gst_element_factory_make(config_.codec(), NULL);
        assert(encoder_);
    }
}



void AudioSender::init_sink()
{
    if (config_.isNetworked()) 
    {
        payloader_ = gst_element_factory_make("rtpvorbispay", NULL);
        assert(payloader_);

        gst_bin_add_many(GST_BIN(pipeline_), encoder_, payloader_, NULL);
        assert(gst_element_link_many(interleave_, encoder_, payloader_, NULL));

        init_rtp();
#if 0 
        sink_ = gst_element_factory_make("udpsink", NULL);
        assert(sink_);
    
        g_object_set(G_OBJECT(sink_), "host", config_.remoteHost(), "port", config_.port(), NULL);

        gst_bin_add_many(GST_BIN(pipeline_), encoder_, payloader_, sink_, NULL);
        assert(gst_element_link_many(interleave_, encoder_, payloader_, sink_, NULL));
#endif
    }
    else // local version
    {
        sink_ = gst_element_factory_make("jackaudiosink", NULL);
        assert(sink_);
        g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

        gst_bin_add(GST_BIN(pipeline_), sink_);
        assert(gst_element_link_many(interleave_, sink_, NULL));
    }
}



// FIXME: Rtp stuff is important/big/used enough to be in its own class
void AudioSender::init_rtp()
{
    rtpbin_ = gst_element_factory_make("gstrtpbin", NULL);
    assert(rtpbin_);

    rtp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtp_sender_);
    //TODO: ts-offset
	g_object_set(rtp_sender_, "host", config_.remoteHost(), "port", config_.port(), "sync", FALSE, "async", FALSE, NULL); 

    rtcp_sender_ = gst_element_factory_make("udpsink", NULL);
    assert(rtcp_sender_);
	g_object_set(rtcp_sender_, "host", config_.remoteHost(), "port", config_.port() + 1, "sync", FALSE, "async", FALSE, NULL); 

    rtcp_receiver_ = gst_element_factory_make("udpsrc", NULL);
    assert(rtcp_receiver_);
	g_object_set(rtcp_receiver_, "port", config_.port() + 5, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), rtpbin_, rtp_sender_, rtcp_sender_, rtcp_receiver_, NULL);
	
    GstPad *send_rtp_sink = gst_element_get_request_pad(rtpbin_, "send_rtp_sink_0");
    assert(send_rtp_sink);
	GstPad *send_rtp_src = gst_element_get_static_pad(rtpbin_, "send_rtp_src_0");
    assert(send_rtp_src);
	GstPad *send_rtcp_src = gst_element_get_request_pad(rtpbin_, "send_rtcp_src_0");
    assert(send_rtcp_src);
	GstPad *recv_rtcp_sink = gst_element_get_request_pad(rtpbin_, "recv_rtcp_sink_0");
    assert(recv_rtcp_sink);

    GstPad *payloadSrc = gst_element_get_static_pad(payloader_, "src");
    assert(payloadSrc);
    GstPad *rtpSenderSink = gst_element_get_static_pad(rtp_sender_, "sink");
    assert(rtpSenderSink);
    GstPad *rtcpSenderSink = gst_element_get_static_pad(rtcp_sender_, "sink");
    assert(rtcpSenderSink);
    GstPad *rtcpReceiverSrc = gst_element_get_static_pad(rtcp_receiver_, "src");
    assert(rtcpReceiverSrc);

    // link pads
    assert(gst_pad_link(payloadSrc, send_rtp_sink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(send_rtp_src, rtpSenderSink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(send_rtcp_src, rtcpSenderSink) == GST_PAD_LINK_OK);
    assert(gst_pad_link(rtcpReceiverSrc, recv_rtcp_sink) == GST_PAD_LINK_OK);

    // release static pads (in reverse order)
    gst_object_unref(GST_OBJECT(rtcpReceiverSrc));
    gst_object_unref(GST_OBJECT(rtcpSenderSink));
    gst_object_unref(GST_OBJECT(rtpSenderSink));
    gst_object_unref(GST_OBJECT(payloadSrc));

    // release request and static pads (in reverse order)
    gst_element_release_request_pad(rtpbin_, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin_, send_rtcp_src);
    gst_object_unref(GST_OBJECT(send_rtp_src));
    gst_element_release_request_pad(rtpbin_, send_rtp_sink);
}
    

// returns caps for last sink, needs to be sent to receiver for rtpvorbisdepay
const char * AudioSender::caps_str() const
{
    assert(isPlaying());
    
    GstPad *pad;
    GstCaps *caps;

    pad = gst_element_get_pad(GST_ELEMENT(rtp_sender_), "sink");
    assert(pad); 

    do
        caps = gst_pad_get_negotiated_caps(pad);
    while (caps == NULL);
    assert(caps != NULL);

    gst_object_unref(pad);

    const char * result = gst_caps_to_string(caps);
    gst_caps_unref(caps); 
    return result; 
}



void AudioSender::send_caps() const
{
    LOG("Sending caps...");

    lo_address t = lo_address_new(NULL, "7770");
    if (lo_send(t, "/audio/rx/caps", "s", caps_str()) == -1)
        std::cerr << "OSC error " << lo_address_errno(t) << ": " << lo_address_errstr(t) << std::endl;
}



// set channel layout for interleave element
void AudioSender::set_channel_layout()
{
    GValue val = { 0, };
    GValueArray *arr;       // for channel position layout
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

    if (config_.isNetworked())    
    {
        std::cout << "Sending audio to host " << config_.remoteHost() << " on port " << config_.port() 
            << std::endl;

        wait_until_playing();
        send_caps();   
    }

    return true;
}

// courtesy of vorbisenc.c

const GstAudioChannelPosition AudioSender::VORBIS_CHANNEL_POSITIONS[][8] = {
    {                             /* Mono */
        GST_AUDIO_CHANNEL_POSITION_FRONT_MONO
    },
    {                             /* Stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                             /* Stereo + Centre */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                             /* Quadraphonic */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
    },
    {                             /* Stereo + Centre + rear stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
    },
    {                             /* Full 5.1 Surround */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_LFE,
    },
    {                             /* Not defined by spec, GStreamer default */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE,
        GST_AUDIO_CHANNEL_POSITION_REAR_CENTER,
    },
    {                             /* Not defined by spec, GStreamer default */
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
