
// videoSender.cpp

#include <iostream>
#include <cassert>
#include <gst/gst.h>

#include "videoSender.h"

VideoSender::VideoSender(int port)
{
    GstElement *txSrc, *txSink, *txCsp, *x264enc, *rtph264pay;

    // validate port number
    if (port < 1000)
    {
        std::cerr << "Port is too low, defaulting to 5060";
        port = 5060;
    }
        
    port_ = port;

    /*----------------------------------------------*/ 
    //  Create sender pipeline
    /*----------------------------------------------*/ 
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

    g_object_set(G_OBJECT(x264enc),"bitrate", 1000, NULL);
    g_object_set(G_OBJECT(x264enc),"byte-stream", TRUE, NULL);
    g_object_set(G_OBJECT(x264enc),"threads", 4, NULL);
    
    g_object_set(G_OBJECT(txSink), "host", "localhost", "port", port_, NULL);
    

    gst_bin_add_many(GST_BIN(pipeline_), txSrc, txCsp, x264enc, 
            rtph264pay, txSink, NULL);
 
    // links camera first filter and second filter (csp)
    gst_element_link_many(txSrc, txCsp, x264enc, rtph264pay, txSink, NULL);
}



VideoSender::~VideoSender()
{
    std::cout << "Sender: I'm melting!!!" << std::endl;
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}



void VideoSender::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}



void VideoSender::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
}
