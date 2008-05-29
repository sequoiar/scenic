// videoReceiver.cpp

#include <iostream>
#include <cassert>

#include "videoBase.h"
#include "videoReceiver.h"

#include <gst/gst.h>

VideoReceiver::VideoReceiver() : MediaBase()
{
    // empty
}



VideoReceiver::~VideoReceiver() 
{
    // empty
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

