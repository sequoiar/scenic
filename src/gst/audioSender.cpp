
// audioSender.cpp

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <sstream>
#include <gst/gst.h>

#include <jack/jack.h>

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



bool AudioSender::init(const int port, const std::string addr, const std::string media) 
{
    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;

    remoteHost_ = std::string(addr);

    //  Create sender pipeline
    //  TODO: should these be subclasses?
    if (!media.compare("monoTest"))
    {
        init_mono_test();
        return true;
    }
    else if (!media.compare("stereoTest"))
    {
        init_stereo_test();
        return true;
    }
    else if (!media.compare("multiTest"))
    {
        init_multi_test();
        return true;
    }
    else if (!media.compare("multiRtpTest"))
    {
        init_multi_rtp_test();
        return true;
    }
    else
    {
        std::cout << "Invalid service type." << std::endl;
        return false;
    }
}



void AudioSender::init_mono_test()
{
    numChannels_ = 1;
    GstElement *txSrc1, *aconv1, *txSink1;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

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



void AudioSender::init_stereo_test()
{
    numChannels_ = 2;

    GstElement *interleave, *aconv0, *queue0, *txSink1;
    GstElement *txSrc1, *aconv1, *queue1;
    GstElement *txSrc2, *aconv2, *queue2;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

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



void AudioSender::init_multi_test()
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



void AudioSender::init_multi_rtp_test()
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

    // FIXME: these should be set to private class variables, not hardcoded
    g_object_set(G_OBJECT(txSink1), "host", "localhost", "port", 5060, NULL);

    g_object_set(G_OBJECT(txSrc1), "volume", 0.125, "freq", 200.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc2), "volume", 0.125, "freq", 300.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc3), "volume", 0.125, "freq", 400.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc4), "volume", 0.125, "freq", 500.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc5), "volume", 0.125, "freq", 600.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc6), "volume", 0.125, "freq", 700.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc7), "volume", 0.125, "freq", 800.0, "is-live", TRUE, NULL);
    g_object_set(G_OBJECT(txSrc8), "volume", 0.125, "freq", 900.0, "is-live", TRUE, NULL);
}



#if 0
bool AudioSender::connect_audio()
{
    // make jack output ports and input ports connect, via jack api
    jack_client_t *client;

    /* try to become a client of the JACK server */

    if ((client = jack_client_new ("test_client")) == 0)
    {
        std::cerr << "jack server not running?" << std::endl;
        return false;
    }

    for (int i = 1; i <= numChannels_; i++)
    {
        std::stringstream istream;
        istream << i;
        std::string outputName = "<unknown>:out_txSink" + istream.str() + "_1";
        std::string inputName = "system:playback_" + istream.str();
        std::cout << "Connecting " << outputName << " to " << inputName 
            << std::endl;
        if (jack_connect(client, outputName.c_str(), inputName.c_str()))
        {
            std::cerr << "cannot connect input ports" << std::endl;
            jack_client_close(client);
            return false;
        }
    }
    jack_client_close(client);
    return true;
}

#endif


bool AudioSender::start()
{
    MediaBase::start();
    return true;
}

