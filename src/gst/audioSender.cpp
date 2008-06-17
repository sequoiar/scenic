
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

#include "audioSender.h"


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



AudioSender::AudioSender(const AudioConfig& config) : config_(config)
{
    // empty
}



AudioSender::~AudioSender() 
{
    // empty
}


// pipeline could also be built with parse launch
bool AudioSender::init()
{
    std::vector<GstElement*> sources, aconvs, queues; 
    GstElement *interleave, *encoder, *payloader, *sink;
    GstIter source, aconv, queue;
    

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    if (config_.numChannels() == 1)    // no need for interleave, special case
    {
        sources.push_back(gst_element_factory_make(config_.source(), NULL));
        assert(sources[0]);

        aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs[0]);

        sink = gst_element_factory_make("jackaudiosink", NULL);
        assert(sink);

        g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

        gst_bin_add_many(GST_BIN(pipeline_), sources[0], aconvs[0], sink, NULL);

        assert(gst_element_link_many(sources[0], aconvs[0], sink, NULL));
        return true;
    }

    interleave = gst_element_factory_make("interleave", NULL);
    assert(interleave);

    set_channel_layout(interleave);
    
    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        sources.push_back(gst_element_factory_make(config_.source(), NULL));
        assert(sources[channelIdx]);
        aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs[channelIdx]);
        queues.push_back(gst_element_factory_make("queue", NULL));
        assert(queues[channelIdx]);
    }

    if (config_.isNetworked()) 
    {
        encoder = gst_element_factory_make("vorbisenc", NULL);
        assert(encoder);
        payloader = gst_element_factory_make("rtpvorbispay", NULL);
        assert(payloader);
        sink = gst_element_factory_make("udpsink", NULL);
        assert(sink);
    
        g_object_set(G_OBJECT(sink), "host", config_.remoteHost(), "port", config_.port(), 
                NULL);

        gst_bin_add_many(GST_BIN(pipeline_), interleave, encoder, payloader, sink, NULL);
    }
    else // local version
    {
        sink = gst_element_factory_make("jackaudiosink", NULL);
        assert(sink);
        g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

        gst_bin_add_many(GST_BIN(pipeline_), interleave, sink, NULL);
    }

    for (source = sources.begin(), aconv = aconvs.begin(), queue = queues.begin(); 
         source != sources.end() && aconv != aconvs.end() && queue != queues.end();
         source++, aconv++, queue++)
        gst_bin_add_many(GST_BIN(pipeline_), *source, *aconv, *queue, NULL);

    if (config_.isNetworked()) 
        assert(gst_element_link_many(interleave, encoder, payloader, sink, NULL));
    else
        assert(gst_element_link_many(interleave, sink, NULL));

    for (source = sources.begin(), aconv = aconvs.begin(); 
            source != sources.end() && aconv != aconvs.end(); source++, aconv++)
        assert(gst_element_link_many(*source, *aconv, interleave, NULL));

    init_test_sources(sources);

    return true;
}



void AudioSender::init_test_sources(std::vector<GstElement*> & sources)
{
    const double GAIN = 1.0 / config_.numChannels(); // so sum of tones equals 1.0
    double frequency = 100.0;
    
    for (GstIter iter = sources.begin(); iter != sources.end(); iter++)
    {
        g_object_set(G_OBJECT(*iter), "volume", GAIN, "freq", frequency, "is-live", TRUE, NULL);
        frequency += 100.0;
    }
}


// finds last sink ELEMENT, returns a string representation of its sink pad's caps
const std::string AudioSender::caps_str() const
{
    bool done = false;
    std::string result;
    gpointer gsink;
    GstPad *pad;
    GstCaps *caps;
    GstIterator *it; 

    if (!isPlaying())
    {
        std::cout << "Cannot return caps, pipeline hasn't been started yet.";
        return result;
    }

    // get pipeline's last sink
    it = gst_bin_iterate_sinks(GST_BIN (pipeline_));

    while (!done) 
    {
        switch (gst_iterator_next(it, &gsink)) 
        {
            case GST_ITERATOR_OK:
                pad = gst_element_get_pad(GST_ELEMENT(gsink), "sink");
                std::cout << gst_element_get_name(GST_ELEMENT(gsink)) << std::endl;
                assert(pad); 
                caps = gst_pad_get_negotiated_caps(pad);
                assert(caps);
                result = std::string(gst_caps_to_string(caps));
                gst_object_unref(pad);
                gst_object_unref(gsink);
                done = true;  
                break;
            case GST_ITERATOR_RESYNC:
                gst_iterator_resync(it);
                break;
            case GST_ITERATOR_ERROR:
                done = true;
                break;
            case GST_ITERATOR_DONE:
                done = true;
                break;
        }
    }
    gst_iterator_free(it);

    return result;
}



void AudioSender::set_channel_layout(GstElement *interleave)
{
    GValue val = { 0, };
    GValueArray *arr;       // for channel position layout
    arr = g_value_array_new(config_.numChannels());

    g_object_set(interleave, "channel-positions-from-input", FALSE, NULL);

    g_value_init(&val, GST_TYPE_AUDIO_CHANNEL_POSITION);

    for (int channelIdx = 0; channelIdx < config_.numChannels(); channelIdx++)
    {
        g_value_set_enum(&val, VORBIS_CHANNEL_POSITIONS[config_.numChannels() - 1][channelIdx]);        
        g_value_array_append(arr, &val);
        g_value_reset(&val);
    }
    g_value_unset(&val);

    g_object_set(interleave, "channel-positions", arr, NULL);
    g_value_array_free(arr);
}





#if 0
// FIXME THIS IS AWFUL but works
void AudioSender::init_uncomp_rtp_test(int numChannels)
{
    GError* error = NULL;
    numChannels_ = numChannels;

    std::stringstream port1, port2, port3;
    port1  << port_; 
    port2 << port_ + 1; 
    port3 << port_ + 5;

    std::string launchStr = " gstrtpbin name=rtpbin \\ " 
        "audiotestsrc ! audioconvert ! alawenc ! rtppcmapay ! rtpbin.send_rtp_sink_0  \\ "
        "rtpbin.send_rtp_src_0 ! udpsink port=" + port1.str() + " host=localhost \\ "
        "rtpbin.send_rtcp_src_0 ! udpsink port=" + port2.str() + 
        " host=localhost sync=false async=false \\ "
        "udpsrc port=" + port3.str() + " ! rtpbin.recv_rtcp_sink_0";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
    make_verbose();

#if 0
    GstElement *rtpbin;
    GstElement *audioSrc1, *aconv1, *encoder, *payloader;
    GstElement *udpSrc1, /**udpSink1,*/ *udpSink2;
    GstPad *send_rtp_sink, *send_rtcp_src, *recv_rtcp_sink, *tempPad;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    // channel 1

    audioSrc1 = gst_element_factory_make("audiotestsrc", "audioSrc1");
    assert(audioSrc1);
    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    encoder = gst_element_factory_make("alawenc", "encoder");
    assert(encoder);
    payloader = gst_element_factory_make("rtppcmapay", "payloader");
    assert(payloader);

    // Transmission

    rtpbin = gst_element_factory_make("gstrtpbin", "rtpbin");
    assert(rtpbin);
    udpSrc1 = gst_element_factory_make("udpsrc", "udpSrc1"); // for tcp
    assert(udpSrc1);
    udpSink1_ = gst_element_factory_make("udpsink", "udpSink1");
    assert(udpSink1_);
    udpSink2 = gst_element_factory_make("udpsink", "udpSink2");
    assert(udpSink2);

    // get pads from rtpbin
    send_rtp_sink = gst_element_get_request_pad(rtpbin, "send_rtp_sink_0");
    assert(send_rtp_sink);

    // FIXME: this pad is only available SOMETIMES, so it has to be added dynamically via a callback
    //send_rtp_src = gst_element_get_request_pad(rtpbin, "send_rtp_src_%d");
    //assert(send_rtp_src);

    send_rtcp_src = gst_element_get_request_pad(rtpbin, "send_rtcp_src_0"); 
    assert(send_rtcp_src);
    recv_rtcp_sink = gst_element_get_request_pad(rtpbin, "recv_rtcp_sink_0"); 
    assert(recv_rtcp_sink);

    // end of channels
    gst_bin_add_many(GST_BIN(pipeline_), rtpbin, audioSrc1, aconv1, encoder, payloader, udpSink1_, 
            udpSink2, udpSrc1, NULL);

    // links transmission line, and audiotestsrcs
    gst_element_link_many(audioSrc1, aconv1, encoder, payloader, NULL);

    // link rtpbin pads
    tempPad = gst_element_get_pad(payloader, "src");
    assert(tempPad);
    gst_pad_link(tempPad, send_rtp_sink);
    gst_object_unref(GST_OBJECT(tempPad));

    // tempPad = gst_element_get_pad(udpSink1, "sink");
    // gst_pad_link(send_rtp_src, tempPad);
    // gst_object_unref(GST_OBJECT(tempPad));

    tempPad = gst_element_get_pad(udpSink2, "sink");
    assert(tempPad);
    gst_pad_link(send_rtcp_src, tempPad);
    gst_object_unref(GST_OBJECT(tempPad));

    tempPad = gst_element_get_pad(udpSrc1, "src");
    assert(tempPad);
    gst_pad_link(tempPad, recv_rtcp_sink);
    gst_object_unref(GST_OBJECT(tempPad));

    // release requested pads in reverse order
    gst_element_release_request_pad(rtpbin, recv_rtcp_sink);
    gst_element_release_request_pad(rtpbin, send_rtcp_src);
    //gst_element_release_request_pad(rtpbin, send_rtp_src);
    gst_element_release_request_pad(rtpbin, send_rtp_sink);

    // FIXME: host ip should be a private member
    g_object_set(G_OBJECT(udpSink1_), "host", "localhost", "port", port_, NULL);
    g_object_set(G_OBJECT(udpSink2), "host", "localhost", "port", port_ + 1, "sync", FALSE, 
            "async", FALSE, NULL);
    g_object_set(G_OBJECT(udpSrc1), "port", port_ + 5, NULL);

    g_object_set(G_OBJECT(audioSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);

    // connect signal for when pad is added
    g_signal_connect(rtpbin, "pad-added", G_CALLBACK(cb_new_pad), (void *) this);
#endif
}
#endif



bool AudioSender::start()
{
    std::cout << "Sending audio to host " << config_.remoteHost() << " on port " << config_.port() << std::endl;
    MediaBase::start();
    return true;
}

