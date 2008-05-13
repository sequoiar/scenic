// videoReceiver.cpp

#include <iostream>
#include <gst/gst.h>

#include "videoReceiver.h"

VideoReceiver::VideoReceiver(int port)
{
    GstElement *rxSrc, *ffdec_h264, *rtph264depay, *rxSink;
    GstCaps *caps;

    // validate port number
    if (port < 1000)
    {
        std::cerr << "Port is too low, defaulting to 5060";
        port = 5060;
    }
        
    port_ = port;

/*----------------------------------------------*/ 
//  Create receiver pipeline
/*----------------------------------------------*/ 
    pipeline_ = gst_pipeline_new("txPipeline");
    if (!pipeline_)
        std::cerr << "Pipeline is bogus." << std::endl;

    rxSrc = gst_element_factory_make("udpsrc", "rxSrc");
    if (!rxSrc)
        std::cerr << "Src is bogus." << std::endl;

    rtph264depay = gst_element_factory_make("rtph264depay", "rtph264depay");
    if (!rtph264depay)
        std::cerr << "rtph264depay is bogus." << std::endl;

    ffdec_h264 = gst_element_factory_make("ffdec_h264", "ffdec_h264");
    if (!ffdec_h264)
        std::cerr << "ffdec_h264 is bogus." << std::endl;

    rxSink = gst_element_factory_make("xvimagesink", "rxSink");
    if (!rxSink)
        std::cerr << "rxSink is bogus." << std::endl;
    
    gst_bin_add_many(GST_BIN(pipeline_), rxSrc, rtph264depay, 
                        ffdec_h264, rxSink, NULL); 
 
    caps = gst_caps_new_simple("application/x-rtp", NULL);
    if (!caps)
        std::cerr << "caps are bogus." << std::endl;

    g_object_set(G_OBJECT(rxSrc), "caps", caps, NULL);
    g_object_set(G_OBJECT(rxSrc), "port", port, NULL);
    g_object_set(G_OBJECT(rxSink), "sync", FALSE, NULL);

    gst_element_link_many(rxSrc, rtph264depay, ffdec_h264, rxSink, NULL);
}



VideoReceiver::~VideoReceiver()
{
    stop();
    gst_object_unref(GST_OBJECT(pipeline_));
}



void VideoReceiver::start()
{
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}



void VideoReceiver::stop()
{
    gst_element_set_state(pipeline_, GST_STATE_NULL);
}
