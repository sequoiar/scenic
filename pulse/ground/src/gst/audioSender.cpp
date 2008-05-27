
// audioSender.cpp

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <gst/gst.h>

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
    if (!media.compare("test"))
    {
            initTest();
            return true;
    }
    else
    {
        std::cout << "Invalid service type." << std::endl;
        return false;
    }
}



void AudioSender::initTest()
{
    GstElement *txSrc, *aconv, *txSink;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    txSrc = gst_element_factory_make("audiotestsrc", "txSrc");
    assert(txSrc);

    aconv = gst_element_factory_make("audioconvert", "aconv");
    assert(aconv);

    txSink = gst_element_factory_make("jackaudiosink", "txSink");
    assert(txSink);

    g_object_set(G_OBJECT(txSink), "connect", 0, NULL);
    
    gst_bin_add_many(GST_BIN(pipeline_), txSrc, aconv, txSink, NULL);
 
    // links testsrc, audio converter, and jack sink
    gst_element_link_many(txSrc, aconv, txSink, NULL);
}

void AudioSender::start()
{
    std::cout << "NOT Sending audio on port " << port_ << " to host " << remoteHost_
        << std::endl;

    AudioBase::start();
}
