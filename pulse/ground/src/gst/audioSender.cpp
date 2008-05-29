
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
        initMonoTest();
        return true;
    }
    else if (!media.compare("stereoTest"))
    {
        initStereoTest();
        return true;
    }
    else if (!media.compare("multiTest"))
    {
        initMultiTest();
        return true;
    }
    else
    {
        std::cout << "Invalid service type." << std::endl;
        return false;
    }
}



void AudioSender::initMonoTest()
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

    g_object_set(G_OBJECT(txSink1), "connect", 0, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), txSrc1, aconv1, txSink1, NULL);

    // links testsrc, audio converter, and jack sink
    gst_element_link_many(txSrc1, aconv1, txSink1, NULL);

}



void AudioSender::initStereoTest()
{
    numChannels_ = 2;
    GstElement *txSrc1, *aconv1, *txSink1;
    GstElement *txSrc2, *aconv2, *txSink2;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    // left channel

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);

    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);

    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1");
    assert(txSink1);

    // right channel 

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);

    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);

    txSink2 = gst_element_factory_make("jackaudiosink", "txSink2");
    assert(txSink2);

    g_object_set(G_OBJECT(txSink1), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink2), "connect", 0, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), txSrc1, aconv1, txSink1, 
            txSrc2, aconv2, txSink2, NULL);

    // links testsrc, audio converter, and jack sink
    gst_element_link_many(txSrc1, aconv1, txSink1, NULL);
    gst_element_link_many(txSrc2, aconv2, txSink2, NULL);

}



void AudioSender::initMultiTest()
{
    numChannels_ = 8;

    GstElement *txSrc1, *aconv1, *txSink1;
    GstElement *txSrc2, *aconv2, *txSink2;
    GstElement *txSrc3, *aconv3, *txSink3;
    GstElement *txSrc4, *aconv4, *txSink4;
    GstElement *txSrc5, *aconv5, *txSink5;
    GstElement *txSrc6, *aconv6, *txSink6;
    GstElement *txSrc7, *aconv7, *txSink7;
    GstElement *txSrc8, *aconv8, *txSink8;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    // channel 1

    txSrc1 = gst_element_factory_make("audiotestsrc", "txSrc1");
    assert(txSrc1);
    aconv1 = gst_element_factory_make("audioconvert", "aconv1");
    assert(aconv1);
    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1");
    assert(txSink1);

    // channel 2

    txSrc2 = gst_element_factory_make("audiotestsrc", "txSrc2");
    assert(txSrc2);
    aconv2 = gst_element_factory_make("audioconvert", "aconv2");
    assert(aconv2);
    txSink2 = gst_element_factory_make("jackaudiosink", "txSink2");
    assert(txSink2);

    // channel 3

    txSrc3 = gst_element_factory_make("audiotestsrc", "txSrc3");
    assert(txSrc3);
    aconv3 = gst_element_factory_make("audioconvert", "aconv3");
    assert(aconv3);
    txSink3 = gst_element_factory_make("jackaudiosink", "txSink3");
    assert(txSink3);

    // channel 4

    txSrc4 = gst_element_factory_make("audiotestsrc", "txSrc4");
    assert(txSrc4);
    aconv4 = gst_element_factory_make("audioconvert", "aconv4");
    assert(aconv4);
    txSink4 = gst_element_factory_make("jackaudiosink", "txSink4");
    assert(txSink4);

    // channel 5

    txSrc5 = gst_element_factory_make("audiotestsrc", "txSrc5");
    assert(txSrc5);
    aconv5 = gst_element_factory_make("audioconvert", "aconv5");
    assert(aconv5);
    txSink5 = gst_element_factory_make("jackaudiosink", "txSink5");
    assert(txSink5);

    // channel 6

    txSrc6 = gst_element_factory_make("audiotestsrc", "txSrc6");
    assert(txSrc6);
    aconv6 = gst_element_factory_make("audioconvert", "aconv6");
    assert(aconv6);
    txSink6 = gst_element_factory_make("jackaudiosink", "txSink6");
    assert(txSink6);

    // channel 7

    txSrc7 = gst_element_factory_make("audiotestsrc", "txSrc7");
    assert(txSrc7);
    aconv7 = gst_element_factory_make("audioconvert", "aconv7");
    assert(aconv7);
    txSink7 = gst_element_factory_make("jackaudiosink", "txSink7");
    assert(txSink7);

    // channel 8

    txSrc8 = gst_element_factory_make("audiotestsrc", "txSrc8");
    assert(txSrc8);
    aconv8 = gst_element_factory_make("audioconvert", "aconv8");
    assert(aconv8);
    txSink8 = gst_element_factory_make("jackaudiosink", "txSink8");
    assert(txSink8);

    g_object_set(G_OBJECT(txSink1), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink2), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink3), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink4), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink5), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink6), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink7), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink8), "connect", 0, NULL);

    gst_bin_add_many(GST_BIN(pipeline_), txSrc1, aconv1, txSink1, 
            txSrc2, aconv2, txSink2, txSrc3, aconv3, txSink3,
            txSrc4, aconv4, txSink4, txSrc5, aconv5, txSink5, 
            txSrc6, aconv6, txSink6, txSrc7, aconv7, txSink7, 
            txSrc8, aconv8, txSink8, NULL);

    // links testsrc, audio converter, and jack sink
    gst_element_link_many(txSrc1, aconv1, txSink1, NULL);
    gst_element_link_many(txSrc2, aconv2, txSink2, NULL);
    gst_element_link_many(txSrc3, aconv3, txSink3, NULL);
    gst_element_link_many(txSrc4, aconv4, txSink4, NULL);
    gst_element_link_many(txSrc5, aconv5, txSink5, NULL);
    gst_element_link_many(txSrc6, aconv6, txSink6, NULL);
    gst_element_link_many(txSrc6, aconv6, txSink6, NULL);
    gst_element_link_many(txSrc7, aconv7, txSink7, NULL);
    gst_element_link_many(txSrc8, aconv8, txSink8, NULL);
}



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
        std::cout << "Connecting " << outputName << " to " << inputName << std::endl;
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



bool AudioSender::start()
{
    MediaBase::start();

    std::cout << "About to connect ports, enter key to continue" << std::endl;
    char c;
    std::cin >> c;
    if (connect_audio())
    {
        std::cout << "Audio connected successfully!" << std::endl;
        return true;
    }
    else
    {
        std::cout << "Error connecting audio" << std::endl;
        return false;
    }
}

