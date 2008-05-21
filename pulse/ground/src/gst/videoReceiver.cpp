// videoReceiver.cpp

#include <iostream>
#include <cassert>
#include <gst/gst.h>

#include "videoReceiver.h"

const int VideoReceiver::DEF_PORT = 10010;

VideoReceiver::VideoReceiver() : isPlaying_(false)
{
}



bool VideoReceiver::init(int port)
{
    if (port < 1000)
        port_ = DEF_PORT;
    else
        port_ = port;

    //  Create receiver pipeline
    GstElement *rxSrc, *ffdec_h264, *rtph264depay, *rxSink;
    GstCaps *caps;

    pipeline_ = gst_pipeline_new("txPipeline");
    assert(pipeline_);

    rxSrc = gst_element_factory_make("udpsrc", "rxSrc");
    assert(rxSrc);

    rtph264depay = gst_element_factory_make("rtph264depay", "rtph264depay");
    assert(rtph264depay);

    ffdec_h264 = gst_element_factory_make("ffdec_h264", "ffdec_h264");
    assert(ffdec_h264);

    rxSink = gst_element_factory_make("xvimagesink", "rxSink");
    assert(rxSink);

    gst_bin_add_many(GST_BIN(pipeline_), rxSrc, rtph264depay, 
            ffdec_h264, rxSink, NULL); 

    caps = gst_caps_new_simple("application/x-rtp", NULL);
    assert(caps);

    g_object_set(G_OBJECT(rxSrc), "caps", caps, NULL);
    g_object_set(G_OBJECT(rxSrc), "port", port_, NULL);
    g_object_set(G_OBJECT(rxSink), "sync", FALSE, NULL);

    std::cout << "Receiving media on port : " << port_ << std::endl;
    gst_element_link_many(rxSrc, rtph264depay, ffdec_h264, rxSink, NULL);

    return true;
}



VideoReceiver::~VideoReceiver()
{
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}



void VideoReceiver::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    isPlaying_ = true;
}



bool VideoReceiver::isPlaying()
{
    return isPlaying_;
}



void VideoReceiver::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    isPlaying_ = false;
}
