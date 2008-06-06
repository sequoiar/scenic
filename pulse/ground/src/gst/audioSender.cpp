
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


#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <sstream>

#include <gst/gst.h>
//#include <gst/audio/multichannel.h>
//#include <gst/audio/multichannel-enumtypes.h>
#include "mediaBase.h"
#include "audioSender.h"


AudioSender::AudioSender() : MediaBase()
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
    //  TODO: should these just be separate public methods?
    if (!media.compare("1chTest"))
    {
        init_1ch_test();
        return true;
    }
    else if (!media.compare("2chTest"))
    {
        init_2ch_test();
        return true;
    }
    else if (!media.compare("6chTest"))
    {
        init_6ch_test();
        return true;
    }
    else if (!media.compare("8chTest"))
    {
        init_8ch_test();
        return true;
    }
    else if (!media.compare("2chCompRtpTest"))
    {
        init_2ch_comp_rtp_test();
        return true;
    }
    else if (!media.compare("8chCompRtpTest"))
    {
        init_8ch_comp_rtp_test();
        return true;
    }
    else if (!media.compare("8chUncompRtpTest"))
    {
        init_8ch_uncomp_rtp_test();
        return true;
    }
    else
    {
        std::cout << "Invalid service type " << media << std::endl;
        return false;
    }
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



void AudioSender::init_2ch_test()
{
    numChannels_ = 2;

    GstElement *interleave, *aconv0, *queue0, *txSink1;
    GstElement *txSrc1, *aconv1, *queue1;
    GstElement *txSrc2, *aconv2, *queue2;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);
    
    make_verbose();

    // main line

    interleave = gst_element_factory_make("interleave", "interleave");
    assert(interleave);
    aconv0 = gst_element_factory_make("audioconvert", "aconv0"); 
    assert(aconv0);
    queue0 = gst_element_factory_make("queue", "queue0"); 
    assert(queue0);
    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1"); 
    assert(txSink1);

    // left channel

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);

    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    
    queue1 = gst_element_factory_make("queue", "queue1"); 
    assert(queue1);

    // right channel 

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);

    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);

    queue2 = gst_element_factory_make("queue", "queue2"); 
    assert(queue2);

    gst_bin_add_many(GST_BIN(pipeline_), interleave, aconv0, queue0, txSink1, txSrc1, aconv1, queue1, 
            txSrc2, aconv2, queue2, NULL);

    // links testsrc, audio converter, and jack sink
    gst_element_link_many(interleave, aconv0, queue0, txSink1, NULL);
    gst_element_link_many(txSrc1, aconv1, queue1, interleave, NULL);
    gst_element_link_many(txSrc2, aconv2, queue2, interleave, NULL);
    
    // set element params 
    g_object_set(G_OBJECT(txSink1), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(txSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc2), "volume", 0.125, "freq", 300.0, "is-live", TRUE, NULL);
}



void AudioSender::init_6ch_test()
{
    numChannels_ = 6;

    GstElement *interleave, *aconv0, *queue0, *txSink1;
    GstElement *txSrc1, *aconv1, *queue1;
    GstElement *txSrc2, *aconv2, *queue2;
    GstElement *txSrc3, *aconv3, *queue3;
    GstElement *txSrc4, *aconv4, *queue4;
    GstElement *txSrc5, *aconv5, *queue5;
    GstElement *txSrc6, *aconv6, *queue6;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);
    
    make_verbose();

    // Main pipeline
    interleave = gst_element_factory_make("interleave", "interleave");
    assert(interleave);
    aconv0 = gst_element_factory_make("audioconvert", "aconv0");
    assert(aconv0);
    queue0 = gst_element_factory_make("queue", "queue0");
    assert(queue0);
    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1");
    assert(txSink1);


    // channel 1

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);
    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    queue1 = gst_element_factory_make("queue", "queue1");
    assert(queue1);

    // channel 2

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);
    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);
    queue2 = gst_element_factory_make("queue", "queue2");
    assert(queue2);

    // CHannel 3

    txSrc3 = gst_element_factory_make("audiotestsrc", "txSrc3");
    assert(txSrc3);
    aconv3 = gst_element_factory_make("audioconvert", "aconv3");
    assert(aconv3);
    queue3 = gst_element_factory_make("queue", "queue3");
    assert(queue3);

    // channel 4

    txSrc4 = gst_element_factory_make("audiotestsrc", "txSrc4");
    assert(txSrc4);
    aconv4 = gst_element_factory_make("audioconvert", "aconv4");
    assert(aconv4);
    queue4 = gst_element_factory_make("queue", "queue4");
    assert(queue4);

    // channel 5

    txSrc5 = gst_element_factory_make("audiotestsrc", "txSrc5");
    assert(txSrc5);
    aconv5 = gst_element_factory_make("audioconvert", "aconv5");
    assert(aconv5);
    queue5 = gst_element_factory_make("queue", "queue5");
    assert(queue5);

    // channel 6

    txSrc6 = gst_element_factory_make("audiotestsrc", "txSrc6");
    assert(txSrc6);
    aconv6 = gst_element_factory_make("audioconvert", "aconv6");
    assert(aconv6);
    queue6 = gst_element_factory_make("queue", "queue6");
    assert(queue6);


    gst_bin_add_many(GST_BIN(pipeline_), 
            interleave, aconv0, queue0, txSink1, 
            txSrc1, aconv1, queue1, 
            txSrc2, aconv2, queue2, 
            txSrc3, aconv3, queue3,
            txSrc4, aconv4, queue4, 
            txSrc5, aconv5, queue5, 
            txSrc6, aconv6, queue6, NULL);

    // links testsrc, audio converter, and jack sink
    gst_element_link_many(interleave, aconv0, queue0, txSink1, NULL);
    gst_element_link_many(txSrc1, aconv1, queue1, interleave, NULL);
    gst_element_link_many(txSrc2, aconv2, queue2, interleave, NULL);
    gst_element_link_many(txSrc3, aconv3, queue3, interleave, NULL);
    gst_element_link_many(txSrc4, aconv4, queue4, interleave, NULL);
    gst_element_link_many(txSrc5, aconv5, queue5, interleave, NULL);
    gst_element_link_many(txSrc6, aconv6, queue6, interleave, NULL);
    
    // set properties for elements
    g_object_set(G_OBJECT(txSink1), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(txSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc2), "volume", 0.125, "freq", 300.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc3), "volume", 0.125, "freq", 400.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc4), "volume", 0.125, "freq", 500.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc5), "volume", 0.125, "freq", 600.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc6), "volume", 0.125, "freq", 700.0, "is-live", TRUE, NULL);
}



void AudioSender::init_8ch_test()
{
    numChannels_ = 8;

    GstElement *interleave, *aconv0, *queue0, *txSink1;
    GstElement *txSrc1, *aconv1, *queue1;
    GstElement *txSrc2, *aconv2, *queue2;
    GstElement *txSrc3, *aconv3, *queue3;
    GstElement *txSrc4, *aconv4, *queue4;
    GstElement *txSrc5, *aconv5, *queue5;
    GstElement *txSrc6, *aconv6, *queue6;
    GstElement *txSrc7, *aconv7, *queue7; 
    GstElement *txSrc8, *aconv8, *queue8; 

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);
    
    make_verbose();

    // Main pipeline
    interleave = gst_element_factory_make("interleave", "interleave");
    assert(interleave);
    aconv0 = gst_element_factory_make("audioconvert", "aconv0");
    assert(aconv0);
    queue0 = gst_element_factory_make("queue", "queue0");
    assert(queue0);
    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1");
    assert(txSink1);


    // channel 1

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);
    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    queue1 = gst_element_factory_make("queue", "queue1");
    assert(queue1);

    // channel 2

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);
    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);
    queue2 = gst_element_factory_make("queue", "queue2");
    assert(queue2);

    // channel 3

    txSrc3 = gst_element_factory_make("audiotestsrc", "txSrc3");
    assert(txSrc3);
    aconv3 = gst_element_factory_make("audioconvert", "aconv3");
    assert(aconv3);
    queue3 = gst_element_factory_make("queue", "queue3");
    assert(queue3);

    // channel 4

    txSrc4 = gst_element_factory_make("audiotestsrc", "txSrc4");
    assert(txSrc4);
    aconv4 = gst_element_factory_make("audioconvert", "aconv4");
    assert(aconv4);
    queue4 = gst_element_factory_make("queue", "queue4");
    assert(queue4);

    // channel 5

    txSrc5 = gst_element_factory_make("audiotestsrc", "txSrc5");
    assert(txSrc5);
    aconv5 = gst_element_factory_make("audioconvert", "aconv5");
    assert(aconv5);
    queue5 = gst_element_factory_make("queue", "queue5");
    assert(queue5);

    // channel 6

    txSrc6 = gst_element_factory_make("audiotestsrc", "txSrc6");
    assert(txSrc6);
    aconv6 = gst_element_factory_make("audioconvert", "aconv6");
    assert(aconv6);
    queue6 = gst_element_factory_make("queue", "queue6");
    assert(queue6);

    // channel 7

    txSrc7 = gst_element_factory_make("audiotestsrc", "txSrc7");
    assert(txSrc7);
    aconv7 = gst_element_factory_make("audioconvert", "aconv7");
    assert(aconv7);
    queue7 = gst_element_factory_make("queue", "queue7");
    assert(queue7);

    // channel 8

    txSrc8 = gst_element_factory_make("audiotestsrc", "txSrc8");
    assert(txSrc8);
    aconv8 = gst_element_factory_make("audioconvert", "aconv8");
    assert(aconv8);
    queue8 = gst_element_factory_make("queue", "queue8");
    assert(queue8);

    gst_bin_add_many(GST_BIN(pipeline_), 
            interleave, aconv0, queue0, txSink1, 
            txSrc1, aconv1, queue1, 
            txSrc2, aconv2, queue2, 
            txSrc3, aconv3, queue3,
            txSrc4, aconv4, queue4, 
            txSrc5, aconv5, queue5, 
            txSrc6, aconv6, queue6, 
            txSrc7, aconv7, queue7, 
            txSrc8, aconv8, queue8, NULL);

    // links testsrc, audio converter, and jack sink
    gst_element_link_many(interleave, aconv0, queue0, txSink1, NULL);
    gst_element_link_many(txSrc1, aconv1, queue1, interleave, NULL);
    gst_element_link_many(txSrc2, aconv2, queue2, interleave, NULL);
    gst_element_link_many(txSrc3, aconv3, queue3, interleave, NULL);
    gst_element_link_many(txSrc4, aconv4, queue4, interleave, NULL);
    gst_element_link_many(txSrc5, aconv5, queue5, interleave, NULL);
    gst_element_link_many(txSrc6, aconv6, queue6, interleave, NULL);
    gst_element_link_many(txSrc7, aconv7, queue7, interleave, NULL);
    gst_element_link_many(txSrc8, aconv8, queue8, interleave, NULL);
    
    // set properties for elements
    g_object_set(G_OBJECT(txSink1), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(txSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc2), "volume", 0.125, "freq", 300.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc3), "volume", 0.125, "freq", 400.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc4), "volume", 0.125, "freq", 500.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc5), "volume", 0.125, "freq", 600.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc6), "volume", 0.125, "freq", 700.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc7), "volume", 0.125, "freq", 800.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc8), "volume", 0.125, "freq", 900.0, "is-live", TRUE, NULL);
}



void AudioSender::init_2ch_comp_rtp_test()
{
    numChannels_ = 2;

    GstElement *interleave, *encoder, *payloader, *txSink1;
    // layout stuff
    //GValue val = { 0, };
    //GValueArray *arr;
    
    GstElement *txSrc1, *aconv1, *queue1; 
    GstElement *txSrc2, *aconv2, *queue2;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    // Transmission
    
    interleave = gst_element_factory_make("interleave", "interleave");
    assert(interleave);

    encoder = gst_element_factory_make("vorbisenc", "encoder");
    assert(encoder);
    payloader = gst_element_factory_make("rtpvorbispay", "payloader");
    assert(payloader);
    txSink1 = gst_element_factory_make("udpsink", "txSink1");
    assert(txSink1);

    // channel 1

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);
    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    queue1 = gst_element_factory_make("queue", "queue1");
    assert(queue1);

    // channel 2

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);
    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);
    queue2 = gst_element_factory_make("queue", "queue2");
    assert(queue2);

    // end of channels

    gst_bin_add_many(GST_BIN(pipeline_), 
            interleave, encoder, payloader, txSink1, 
            txSrc1, aconv1, queue1, 
            txSrc2, aconv2, queue2,
            NULL);

    // links transmission line, and audiotestsrcs
    gst_element_link_many(interleave, encoder, payloader, txSink1, NULL);
    gst_element_link_many(txSrc1, aconv1, queue1, interleave, NULL);
    gst_element_link_many(txSrc2, aconv2, queue2, interleave, NULL);

    g_object_set(G_OBJECT(txSink1), "host", remoteHost_.c_str(), "port", port_, NULL);

    g_object_set(G_OBJECT(txSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc2), "volume", 0.125, "freq", 300.0, "is-live", TRUE, NULL);
}



void AudioSender::init_8ch_comp_rtp_test()
{
    numChannels_ = 8;

    GstElement *interleave, *encoder, *payloader, *txSink1;
    // layout stuff
    // GValue val = { 0, };
    // GValueArray *arr;
    
    GstElement *txSrc1, *aconv1, *queue1; 
    GstElement *txSrc2, *aconv2, *queue2;
    GstElement *txSrc3, *aconv3, *queue3;
    GstElement *txSrc4, *aconv4, *queue4;
    GstElement *txSrc5, *aconv5, *queue5;
    GstElement *txSrc6, *aconv6, *queue6;
    GstElement *txSrc7, *aconv7, *queue7;
    GstElement *txSrc8, *aconv8, *queue8;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    // Transmission
    
    interleave = gst_element_factory_make("interleave", "interleave");
    assert(interleave);
    
#if 0
    g_object_set(interleave, "channel-positions-from-input", FALSE, NULL);
    arr = g_value_array_new(8);
    g_value_init(&val, GST_TYPE_AUDIO_CHANNEL_POSITION);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT);
    g_value_array_append(arr, &val);
    g_value_reset(&val);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT);
    g_value_array_append(arr, &val);
    g_value_reset(&val);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_REAR_LEFT);
    g_value_array_append(arr, &val);
    g_value_reset(&val);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT);
    g_value_array_append(arr, &val);
    g_value_reset(&val);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_LFE);
    g_value_array_append(arr, &val);
    g_value_reset(&val);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT);
    g_value_array_append(arr, &val);
    g_value_reset(&val);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT);
    g_value_array_append(arr, &val);
    g_value_reset(&val);
    g_value_set_enum(&val, GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER);
    g_value_array_append(arr, &val);
    g_value_unset(&val);
    g_object_set(interleave, "channel-positions", arr, NULL);
    g_value_array_free(arr);
#endif

    encoder = gst_element_factory_make("vorbisenc", "encoder");
    assert(encoder);
    payloader = gst_element_factory_make("rtpvorbispay", "payloader");
    assert(payloader);
    txSink1 = gst_element_factory_make("udpsink", "txSink1");
    assert(txSink1);

    // channel 1

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);
    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    queue1 = gst_element_factory_make("queue", "queue1");
    assert(queue1);

    // channel 2

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);
    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);
    queue2 = gst_element_factory_make("queue", "queue2");
    assert(queue2);

    // channel 3

    txSrc3 = gst_element_factory_make("audiotestsrc", "txSrc3");
    assert(txSrc3);
    aconv3 = gst_element_factory_make("audioconvert", "aconv3");
    assert(aconv3);
    queue3 = gst_element_factory_make("queue", "queue3");
    assert(queue3);

    // channel 4

    txSrc4 = gst_element_factory_make("audiotestsrc", "txSrc4");
    assert(txSrc4);
    aconv4 = gst_element_factory_make("audioconvert", "aconv4");
    assert(aconv4);
    queue4 = gst_element_factory_make("queue", "queue4");
    assert(queue4);

    // channel 5

    txSrc5 = gst_element_factory_make("audiotestsrc", "txSrc5");
    assert(txSrc5);
    aconv5 = gst_element_factory_make("audioconvert", "aconv5");
    assert(aconv5);
    queue5 = gst_element_factory_make("queue", "queue5");
    assert(queue5);

    // channel 6

    txSrc6 = gst_element_factory_make("audiotestsrc", "txSrc6");
    assert(txSrc6);
    aconv6 = gst_element_factory_make("audioconvert", "aconv6");
    assert(aconv6);
    queue6 = gst_element_factory_make("queue", "queue6");
    assert(queue6);

    // channel 7

    txSrc7 = gst_element_factory_make("audiotestsrc", "txSrc7");
    assert(txSrc7);
    aconv7 = gst_element_factory_make("audioconvert", "aconv7");
    assert(aconv7);
    queue7 = gst_element_factory_make("queue", "queue7");
    assert(queue7);

    // channel 8

    txSrc8 = gst_element_factory_make("audiotestsrc", "txSrc8");
    assert(txSrc8);
    aconv8 = gst_element_factory_make("audioconvert", "aconv8");
    assert(aconv8);
    queue8 = gst_element_factory_make("queue", "queue8");
    assert(queue8);

    // end of channels

    gst_bin_add_many(GST_BIN(pipeline_), 
            interleave, encoder, payloader, txSink1, 
            txSrc1, aconv1, queue1, 
            txSrc2, aconv2, queue2,
            txSrc3, aconv3, queue3,
            txSrc4, aconv4, queue4, 
            txSrc5, aconv5, queue5, 
            txSrc6, aconv6, queue6, 
            txSrc7, aconv7, queue7, 
            txSrc8, aconv8, queue8, NULL);

    // links transmission line, and audiotestsrcs
    gst_element_link_many(interleave, encoder, payloader, txSink1, NULL);
    gst_element_link_many(txSrc1, aconv1, queue1, interleave, NULL);
    gst_element_link_many(txSrc2, aconv2, queue2, interleave, NULL);
    gst_element_link_many(txSrc3, aconv3, queue3, interleave, NULL);
    gst_element_link_many(txSrc4, aconv4, queue4, interleave, NULL);
    gst_element_link_many(txSrc5, aconv5, queue5, interleave, NULL);
    gst_element_link_many(txSrc6, aconv6, queue6, interleave, NULL);
    gst_element_link_many(txSrc7, aconv7, queue7, interleave, NULL);
    gst_element_link_many(txSrc8, aconv8, queue8, interleave, NULL);

    g_object_set(G_OBJECT(txSink1), "host", remoteHost_.c_str(), "port", port_, NULL);

    g_object_set(G_OBJECT(txSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc2), "volume", 0.125, "freq", 300.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc3), "volume", 0.125, "freq", 400.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc4), "volume", 0.125, "freq", 500.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc5), "volume", 0.125, "freq", 600.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc6), "volume", 0.125, "freq", 700.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc7), "volume", 0.125, "freq", 800.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc8), "volume", 0.125, "freq", 900.0, "is-live", TRUE, NULL);
}



// not actually uncompressed yet
void AudioSender::init_8ch_uncomp_rtp_test()
{
    numChannels_ = 8;

    GstElement *interleave, *encoder, *payloader, *txSink1;
    GstElement *txSrc1, *aconv1, *queue1; 
    GstElement *txSrc2, *aconv2, *queue2;
    GstElement *txSrc3, *aconv3, *queue3;
    GstElement *txSrc4, *aconv4, *queue4;
    GstElement *txSrc5, *aconv5, *queue5;
    GstElement *txSrc6, *aconv6, *queue6;
    GstElement *txSrc7, *aconv7, *queue7;
    GstElement *txSrc8, *aconv8, *queue8;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    make_verbose();

    // Transmission
    
    interleave = gst_element_factory_make("interleave", "interleave");
    assert(interleave);
    encoder = gst_element_factory_make("alawenc", "encoder");
    assert(encoder);
    payloader = gst_element_factory_make("rtppcmapay", "payloader");
    assert(payloader);
    txSink1 = gst_element_factory_make("udpsink", "txSink1");
    assert(txSink1);

    // channel 1

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);
    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    queue1 = gst_element_factory_make("queue", "queue1");
    assert(queue1);

    // channel 2

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);
    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);
    queue2 = gst_element_factory_make("queue", "queue2");
    assert(queue2);

    // channel 3

    txSrc3 = gst_element_factory_make("audiotestsrc", "txSrc3");
    assert(txSrc3);
    aconv3 = gst_element_factory_make("audioconvert", "aconv3");
    assert(aconv3);
    queue3 = gst_element_factory_make("queue", "queue3");
    assert(queue3);

    // channel 4

    txSrc4 = gst_element_factory_make("audiotestsrc", "txSrc4");
    assert(txSrc4);
    aconv4 = gst_element_factory_make("audioconvert", "aconv4");
    assert(aconv4);
    queue4 = gst_element_factory_make("queue", "queue4");
    assert(queue4);

    // channel 5

    txSrc5 = gst_element_factory_make("audiotestsrc", "txSrc5");
    assert(txSrc5);
    aconv5 = gst_element_factory_make("audioconvert", "aconv5");
    assert(aconv5);
    queue5 = gst_element_factory_make("queue", "queue5");
    assert(queue5);

    // channel 6

    txSrc6 = gst_element_factory_make("audiotestsrc", "txSrc6");
    assert(txSrc6);
    aconv6 = gst_element_factory_make("audioconvert", "aconv6");
    assert(aconv6);
    queue6 = gst_element_factory_make("queue", "queue6");
    assert(queue6);

    // channel 7

    txSrc7 = gst_element_factory_make("audiotestsrc", "txSrc7");
    assert(txSrc7);
    aconv7 = gst_element_factory_make("audioconvert", "aconv7");
    assert(aconv7);
    queue7 = gst_element_factory_make("queue", "queue7");
    assert(queue7);

    // channel 8

    txSrc8 = gst_element_factory_make("audiotestsrc", "txSrc8");
    assert(txSrc8);
    aconv8 = gst_element_factory_make("audioconvert", "aconv8");
    assert(aconv8);
    queue8 = gst_element_factory_make("queue", "queue8");
    assert(queue8);

    // end of channels

    gst_bin_add_many(GST_BIN(pipeline_), 
            interleave, encoder, payloader, txSink1, 
            txSrc1, aconv1, queue1, 
            txSrc2, aconv2, queue2,
            txSrc3, aconv3, queue3,
            txSrc4, aconv4, queue4, 
            txSrc5, aconv5, queue5, 
            txSrc6, aconv6, queue6, 
            txSrc7, aconv7, queue7, 
            txSrc8, aconv8, queue8, NULL);

    // links transmission line, and audiotestsrcs
    gst_element_link_many(interleave, encoder, payloader, txSink1, NULL);
    gst_element_link_many(txSrc1, aconv1, queue1, interleave, NULL);
    gst_element_link_many(txSrc2, aconv2, queue2, interleave, NULL);
    gst_element_link_many(txSrc3, aconv3, queue3, interleave, NULL);
    gst_element_link_many(txSrc4, aconv4, queue4, interleave, NULL);
    gst_element_link_many(txSrc5, aconv5, queue5, interleave, NULL);
    gst_element_link_many(txSrc6, aconv6, queue6, interleave, NULL);
    gst_element_link_many(txSrc7, aconv7, queue7, interleave, NULL);
    gst_element_link_many(txSrc8, aconv8, queue8, interleave, NULL);

    g_object_set(G_OBJECT(txSink1), "host", remoteHost_.c_str(), "port", port_, NULL);

    g_object_set(G_OBJECT(txSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc2), "volume", 0.125, "freq", 300.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc3), "volume", 0.125, "freq", 400.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc4), "volume", 0.125, "freq", 500.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc5), "volume", 0.125, "freq", 600.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc6), "volume", 0.125, "freq", 700.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc7), "volume", 0.125, "freq", 800.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc8), "volume", 0.125, "freq", 900.0, "is-live", TRUE, NULL);
}



bool AudioSender::start()
{
    std::cout << "Sending audio to host " << remoteHost_ << " on port " << port_ << std::endl;
    MediaBase::start();
    return true;
}

