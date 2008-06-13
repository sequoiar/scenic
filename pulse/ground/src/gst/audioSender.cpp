
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

#include "mediaBase.h"
#include "audioSender.h"


// courtesy of vorbisenc.c

const GstAudioChannelPosition AudioSender::VORBIS_CHANNEL_POSITIONS[][8] = {
    {                             /* Mono */
        GST_AUDIO_CHANNEL_POSITION_FRONT_MONO},
    {                             /* Stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT},
    {                             /* Stereo + Centre */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT},
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

// callback which gets signalled when dynamic pad is created

#if 0
void AudioSender::cb_new_pad(GstElement *element, GstPad *pad, gpointer data)
{
    gchar *name;
    GstPad *tempPad;
    AudioSender *sender = (AudioSender*) data;
    sender->print_address();

    name = gst_pad_get_name (pad);
    g_print ("A new pad %s was created\n", name);
    g_free (name);

    /* here, setup a new pad link for the newly created pad */
   tempPad = gst_element_get_pad(sender->udpSink1_, "sink");
   gst_pad_link(pad, tempPad);
   gst_object_unref(GST_OBJECT(tempPad));
}
#endif



AudioSender::AudioSender() : session_(0, "")
{
    // empty
}



AudioSender::AudioSender(const AudioSession& session) : session_(session)
{
    // empty
}



AudioSender::~AudioSender() 
{
    // empty
}



bool AudioSender::init(const std::string media, const int port, const std::string addr) 
{
    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;

    remoteHost_ = std::string(addr);

    //  Create sender pipeline
    std::cout.flush();
    std::cout << std::endl;
    std::cout << media << std::endl;

    //  FIXME: This is ugly
    if (!media.compare("1chTest"))
    {
        init_local_test(1);
        return true;
    }
    else if (!media.compare("2chTest"))
    {
        init_local_test(2);
        return true;
    }
    else if (!media.compare("6chTest"))
    {
        init_local_test(6);
        return true;
    }
    else if (!media.compare("8chTest"))
    {
        init_local_test(8);
        return true;
    }
    else if (!media.compare("2chCompRtpTest"))
    {
        init_rtp_test(2);
        return true;
    }
    else if (!media.compare("8chCompRtpTest"))
    {
        init_rtp_test(8);
        return true;
    }
    else if (!media.compare("1chUncompRtpTest"))
    {
        init_uncomp_rtp_test(1);
        return true;
    }
    else
    {
        std::cout << "Invalid service type " << media << std::endl;
        return false;
    }
}



bool AudioSender::init()
{
    std::vector<GstElement*> sources, aconvs, queues; 
    GstElement *interleave, *encoder, *payloader, *sink;
    
    // layout stuff
    GValueArray *arr;
    double gain;

    gain = 1.0 / session_.numChannels();

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    if (session_.numChannels() == 1)    // no need for interleave, exception case
    {
        sources.push_back(gst_element_factory_make("audiotestsrc", NULL));
        assert(sources[0]);

        aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs[0]);

        sink = gst_element_factory_make("jackaudiosink", NULL);
        assert(sink);

        g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

        gst_bin_add_many(GST_BIN(pipeline_), sources[0], aconvs[0], sink, NULL);

    // links testsrc, audio converter, and jack sink
        gst_element_link_many(sources[0], aconvs[0], sink, NULL);
        return true;
    }

/*----------------------------------------------*/ 

    interleave = gst_element_factory_make("interleave", NULL);
    assert(interleave);

    g_object_set(interleave, "channel-positions-from-input", FALSE, NULL);
    arr = g_value_array_new(session_.numChannels());
    set_channel_layout(arr);
    g_object_set(interleave, "channel-positions", arr, NULL);
    g_value_array_free(arr);

    for (int channelIdx = 0; channelIdx < numChannels_; channelIdx++)
    {
        sources.push_back(gst_element_factory_make("audiotestsrc", NULL));
        assert(sources[channelIdx]);
        aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs[channelIdx]);
        queues.push_back(gst_element_factory_make("queue", NULL));
        assert(queues[channelIdx]);
    }

    if (session_.port() != 0) // networked
    {
        encoder = gst_element_factory_make("vorbisenc", NULL);
        assert(encoder);
        payloader = gst_element_factory_make("rtpvorbispay", NULL);
        assert(payloader);
        sink = gst_element_factory_make("udpsink", NULL);
        assert(sink);
    
        g_object_set(G_OBJECT(sink), "host", session_.remoteHost().c_str(), "port", session_.port(), 
                NULL);

        gst_bin_add_many(GST_BIN(pipeline_), interleave, encoder, payloader, sink, NULL);
    }
    else
    {
        sink = gst_element_factory_make("jackaudiosink", NULL);
        assert(sink);
        g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

        gst_bin_add_many(GST_BIN(pipeline_), interleave, sink, NULL);
    }

    for (int channelIdx = 0; channelIdx < numChannels_; channelIdx++)
    {
        gst_bin_add_many(GST_BIN(pipeline_), sources[channelIdx], aconvs[channelIdx], 
                queues[channelIdx], NULL);
    }

    if (session_.port() != 0) // networked
    {
        gst_element_link_many(interleave, encoder, payloader, sink, NULL);
    }
    else
        gst_element_link_many(interleave, sink, NULL);

    for (int channelIdx = 0; channelIdx < numChannels_; channelIdx++)
    {
        gst_element_link_many(sources[channelIdx], aconvs[channelIdx], interleave, NULL);
        g_object_set(G_OBJECT(sources[channelIdx]), "volume", gain, "freq", 100.0 * (channelIdx + 1),
                "is-live", TRUE, NULL);
    }

    return true;
#if 0


    // interleave needs its own audioconvert and queue
    interleave = gst_element_factory_make("interleave", NULL);
    assert(interleave);

    aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
    assert(aconvs[0]);

    queues.push_back(gst_element_factory_make("queue", NULL));
    assert(queues[0]);

    for (int channelIdx = 1; channelIdx <= session_.numChannels(); channelIdx++)
    {
        sources.push_back(gst_element_factory_make("audiotestsrc", NULL));
        assert(sources[channelIdx - 1]);
        aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs[channelIdx]);
        queues.push_back(gst_element_factory_make("queue", NULL));
        assert(queues[channelIdx]);
    }

    sink = gst_element_factory_make("jackaudiosink", NULL);
    assert(sink);

    gst_bin_add_many(GST_BIN(pipeline_), interleave, aconvs[0], queues[0], sink, NULL);

    for (int channelIdx = 1; channelIdx <= session_.numChannels(); channelIdx++)
    {
        gst_bin_add(GST_BIN(pipeline_), sources[channelIdx - 1]);
        gst_bin_add(GST_BIN(pipeline_), aconvs[channelIdx]);
        gst_bin_add(GST_BIN(pipeline_), queues[channelIdx]);
    }

    gst_element_link_many(interleave, aconvs[0], queues[0], sink, NULL);

    for (int channelIdx = 1; channelIdx <= session_.numChannels(); channelIdx++)
    {
        gst_element_link_many(sources[channelIdx - 1], aconvs[channelIdx], 
                queues[channelIdx], interleave, NULL);
        g_object_set(G_OBJECT(sources[channelIdx - 1]), "volume", 0.125, "freq", 100.0 * channelIdx, 
                "is-live", TRUE, NULL);
    }

    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

#endif // 0
}



void AudioSender::init_local_test(int numChannels)
{
    std::vector<GstElement*> sources, aconvs, queues; 
    GstElement *interleave, *sink;

    // FIXME: this logic should be moved to init 
    numChannels_ = numChannels;
    if (numChannels_ == 1)
        return init_1ch_test();
    else if (numChannels_ < 2)
        numChannels_ = 2;
    else if (numChannels_ > 8)
        numChannels_ = 8;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    // interleave needs its own audioconvert and queue
    interleave = gst_element_factory_make("interleave", NULL);
    assert(interleave);

    aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
    assert(aconvs[0]);

    queues.push_back(gst_element_factory_make("queue", NULL));
    assert(queues[0]);

    for (int channelIdx = 1; channelIdx <= numChannels_; channelIdx++)
    {
        sources.push_back(gst_element_factory_make("audiotestsrc", NULL));
        assert(sources[channelIdx - 1]);
        aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs[channelIdx]);
        queues.push_back(gst_element_factory_make("queue", NULL));
        assert(queues[channelIdx]);
    }

    sink = gst_element_factory_make("jackaudiosink", NULL);
    assert(sink);

    gst_bin_add_many(GST_BIN(pipeline_), interleave, aconvs[0], queues[0], sink, NULL);

    for (int channelIdx = 1; channelIdx <= numChannels_; channelIdx++)
    {
        gst_bin_add(GST_BIN(pipeline_), sources[channelIdx - 1]);
        gst_bin_add(GST_BIN(pipeline_), aconvs[channelIdx]);
        gst_bin_add(GST_BIN(pipeline_), queues[channelIdx]);
    }

    gst_element_link_many(interleave, aconvs[0], queues[0], sink, NULL);

    for (int channelIdx = 1; channelIdx <= numChannels_; channelIdx++)
    {
        gst_element_link_many(sources[channelIdx - 1], aconvs[channelIdx], 
                queues[channelIdx], interleave, NULL);
        g_object_set(G_OBJECT(sources[channelIdx - 1]), "volume", 0.125, "freq", 100.0 * channelIdx, 
                "is-live", TRUE, NULL);
    }

    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
}



void AudioSender::init_1ch_test()
{
    numChannels_ = 1;
    GstElement *txSrc1, *aconv1, *txSink1;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);

    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);

    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1");
    assert(txSink1);

    g_object_set(G_OBJECT(txSink1), "sync", FALSE, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), txSrc1, aconv1, txSink1, NULL);

    // links testsrc, audio converter, and jack sink
    gst_element_link_many(txSrc1, aconv1, txSink1, NULL);
}



void AudioSender::set_channel_layout(GValueArray *arr)
{
    GValue val = { 0, };
    g_value_init(&val, GST_TYPE_AUDIO_CHANNEL_POSITION);

    for (int channelIdx = 0; channelIdx < numChannels_; channelIdx++)
    {
        g_value_set_enum(&val, VORBIS_CHANNEL_POSITIONS[numChannels_ - 1][channelIdx]);        
        g_value_array_append(arr, &val);
        g_value_reset(&val);
    }
    g_value_unset(&val);
}



void AudioSender::init_rtp_test(int numChannels)
{
    std::vector<GstElement*> sources, aconvs, queues; 
    GstElement *interleave, *encoder, *payloader, *sink;
    // layout stuff
    GValueArray *arr;
    double gain;

    numChannels_ = numChannels;
    if (numChannels_ < 2)
        numChannels_ = 2;
    else if (numChannels_ > 8)
        numChannels_ = 8;

    gain = 1.0 / numChannels_;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    interleave = gst_element_factory_make("interleave", NULL);
    assert(interleave);

    g_object_set(interleave, "channel-positions-from-input", FALSE, NULL);
    arr = g_value_array_new(numChannels_);
    set_channel_layout(arr);
    g_object_set(interleave, "channel-positions", arr, NULL);
    g_value_array_free(arr);

    encoder = gst_element_factory_make("vorbisenc", NULL);
    assert(encoder);
    payloader = gst_element_factory_make("rtpvorbispay", NULL);
    assert(payloader);
    sink = gst_element_factory_make("udpsink", NULL);
    assert(sink);

    for (int channelIdx = 0; channelIdx < numChannels_; channelIdx++)
    {
        sources.push_back(gst_element_factory_make("audiotestsrc", NULL));
        assert(sources[channelIdx]);
        aconvs.push_back(gst_element_factory_make("audioconvert", NULL));
        assert(aconvs[channelIdx]);
        queues.push_back(gst_element_factory_make("queue", NULL));
        assert(queues[channelIdx]);
    }

    gst_bin_add_many(GST_BIN(pipeline_), interleave, encoder, payloader, sink, NULL);

    for (int channelIdx = 0; channelIdx < numChannels_; channelIdx++)
    {
        gst_bin_add_many(GST_BIN(pipeline_), sources[channelIdx], aconvs[channelIdx], 
                queues[channelIdx], NULL);
    }

    gst_element_link_many(interleave, encoder, payloader, sink, NULL);

    for (int channelIdx = 0; channelIdx < numChannels_; channelIdx++)
    {
        gst_element_link_many(sources[channelIdx], aconvs[channelIdx], interleave, NULL);
        g_object_set(G_OBJECT(sources[channelIdx]), "volume", gain, "freq", 100.0 * (channelIdx + 1),
                "is-live", TRUE, NULL);
    }

    g_object_set(G_OBJECT(sink), "host", remoteHost_.c_str(), "port", port_, NULL);
}



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



bool AudioSender::start()
{
    std::cout << "Sending audio to host " << remoteHost_ << " on port " << port_ << std::endl;
    MediaBase::start();
    return true;
}

