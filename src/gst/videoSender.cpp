
// videoSender.cpp

#include <iostream>
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
    if (!pipeline_)
        std::cerr << "Pipeline is bogus." << std::endl;

    txSrc = gst_element_factory_make("videotestsrc", "txSrc");
    if (!txSrc)
        std::cerr << "txSrc is bogus." << std::endl;

    txCsp = gst_element_factory_make("ffmpegcolorspace", "txCsp");
    if (!txCsp)
        std::cerr <<  "csp is bogus." << std::endl;

    x264enc = gst_element_factory_make("x264enc", "x264enc");
    if (!x264enc)
        std::cerr << "x264 is bogus." << std::endl;

    rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    if (!rtph264pay)
        std::cerr << "rtph264pay is bogus." << std::endl;

    txSink = gst_element_factory_make("udpsink", "txSink");
    if (!txSink)
        std::cerr << "Sink is bogus." << std::endl;

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
