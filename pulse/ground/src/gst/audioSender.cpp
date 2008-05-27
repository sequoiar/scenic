
// audioSender.cpp

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <sstream>
#include <gst/gst.h>

#include <jack/jack.h>

#include "audioBase.h"
#include "audioSender.h"


AudioSender::AudioSender() : AudioBase()
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
    else
    {
        std::cout << "Invalid service type." << std::endl;
        return false;
    }
}



void AudioSender::initMonoTest()
{
    numChannels_ = 1;
    GstElement *txSrc, *aconv, *txSink1;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    txSrc = gst_element_factory_make("audiotestsrc", "txSrc");
    assert(txSrc);

    aconv = gst_element_factory_make("audioconvert", "aconv");
    assert(aconv);

    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1");
    assert(txSink1);

    g_object_set(G_OBJECT(txSink1), "connect", 0, NULL);
    
    gst_bin_add_many(GST_BIN(pipeline_), txSrc, aconv, txSink1, NULL);
 
    // links testsrc, audio converter, and jack sink
    gst_element_link_many(txSrc, aconv, txSink1, NULL);

}



void AudioSender::initStereoTest()
{
    numChannels_ = 2;
    GstElement *txSrcLeft, *aconvLeft, *txSink1;
    GstElement *txSrcRight, *aconvRight, *txSink2;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    // left channel

    txSrcLeft = gst_element_factory_make("audiotestsrc", "txSrcLeft");
    assert(txSrcLeft);

    aconvLeft = gst_element_factory_make("audioconvert", "aconvLeft");
    assert(aconvLeft);

    txSink1 = gst_element_factory_make("jackaudiosink", "txSink1");
    assert(txSink1);

    // right channel 
    
    txSrcRight = gst_element_factory_make("audiotestsrc", "txSrcRight");
    assert(txSrcRight);

    aconvRight = gst_element_factory_make("audioconvert", "aconvRight");
    assert(aconvRight);

    txSink2 = gst_element_factory_make("jackaudiosink", "txSink2");
    assert(txSink2);


    g_object_set(G_OBJECT(txSink1), "connect", 0, NULL);
    g_object_set(G_OBJECT(txSink2), "connect", 0, NULL);
    
    gst_bin_add_many(GST_BIN(pipeline_), txSrcLeft, aconvLeft, txSink1, 
            txSrcRight, aconvRight, txSink2, NULL);
 
    // links testsrc, audio converter, and jack sink
    gst_element_link_many(txSrcLeft, aconvLeft, txSink1, NULL);
    gst_element_link_many(txSrcRight, aconvRight, txSink2, NULL);
    
}



void AudioSender::initMultiChannelTest()
{
    numChannels_ = 8;
    // empty 
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



void AudioSender::start()
{
    std::cout << "NOT Sending audio on port " << port_ << " to host " << remoteHost_
        << std::endl;


    AudioBase::start();

    std::cout << "About to connect ports, enter key to continue" << std::endl;
    char c;
    std::cin >> c;
    if (connect_audio())
        std::cout << "Audio connected successfully!" << std::endl;
    else
        std::cout << "Error connecting audio" << std::endl;
}
