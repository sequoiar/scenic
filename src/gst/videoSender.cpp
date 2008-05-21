
// videoSender.cpp

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <gst/gst.h>

#include "videoSender.h"

const int VideoSender::DEF_PORT = 10010;

VideoSender::VideoSender() : isPlaying_(false)
{
    // empty
}



bool VideoSender::init(const int port, const std::string addr, const std::string service) 
{
    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;

    remoteHost_ = std::string(addr);

    //  Create sender pipeline
    if (!service.compare("dv"))
    {
            initDv();
            return true;
    }
    else if (!service.compare("test"))
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



void VideoSender::initDv()
{
    GError *error;
    std::string launchStr = "dv1394src ! dvdemux name=demux demux. ! \
                                  queue ! dvdec ! ffmpegcolorspace \
                                  ! x264enc bitrate=12000 byte-stream=true \
                                  threads=4 ! rtph264pay ! \
                                  udpsink host="; 
                                  
    std::stringstream istream;
    istream << remoteHost_ << " port = " << port_;           
    launchStr += istream.str();     // get port number into launch string
    launchStr += " demux. ! queue ! fakesink";

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);
}



void VideoSender::initTest()
{
    GstElement *txSrc, *txSink, *txCsp, *x264enc, *rtph264pay;

    pipeline_ = gst_pipeline_new("rxPipeline");
    assert(pipeline_);

    txSrc = gst_element_factory_make("videotestsrc", "txSrc");
    assert(txSrc);

    txCsp = gst_element_factory_make("ffmpegcolorspace", "txCsp");
    assert(txCsp);

    x264enc = gst_element_factory_make("x264enc", "x264enc");
    assert(x264enc);

    rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    assert(rtph264pay);

    txSink = gst_element_factory_make("udpsink", "txSink");
    assert(txSink);

    g_object_set(G_OBJECT(x264enc),"bitrate", 1500, NULL);
    g_object_set(G_OBJECT(x264enc),"byte-stream", TRUE, NULL);
    g_object_set(G_OBJECT(x264enc),"threads", 4, NULL);
    
    g_object_set(G_OBJECT(txSink), "host", remoteHost_.c_str(), "port", port_, NULL);
    

    gst_bin_add_many(GST_BIN(pipeline_), txSrc, txCsp, x264enc, rtph264pay, 
                     txSink, NULL);
 
    // links testsrc, colorspace converter, encoder, payloader and udpsink
    gst_element_link_many(txSrc, txCsp, x264enc, rtph264pay, txSink, NULL);
}



VideoSender::~VideoSender()
{
    stop();
    if (pipeline_)
        gst_object_unref(GST_OBJECT(pipeline_));
}



void VideoSender::start()
{
    std::cout << "Sending media on port " << port_ << " to host " << remoteHost_
        << std::endl;
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    isPlaying_ = true;
}



void VideoSender::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    isPlaying_ = false;
}



bool VideoSender::isPlaying()
{
    return isPlaying_;
}
