// gstbuff_ready_cb.c

//#include "gst_sip_rtp.h"
#include <gst/gst.h>

// Prototypes
void gst_run();
void initTxPipeline(GstElement* txPipeline);
void initRxPipeline(GstElement* rxPipeline);

void gst_run()
{
    // create gstreamer components
    GstElement *txPipeline, *rxPipeline; 
    GMainLoop *loop;
    
    // init gstreamer
    gst_init(0, NULL);  // normally should get argc argv
    loop = g_main_loop_new(NULL, FALSE);

    if (!(txPipeline = gst_pipeline_new("txPipeline")))
        fprintf(stdout, "Pipeline is bogus.");
    if (!(rxPipeline = gst_pipeline_new("rxPipeline")))
        fprintf(stdout, "Pipeline is bogus.");

    // TODO: Figure out why this is way slower when order is reversed
    initRxPipeline(rxPipeline);
    initTxPipeline(txPipeline);
    
    // SPIN HERE
    /*----------------------------------------------*/ 
    g_main_loop_run(loop);
    /*----------------------------------------------*/ 

    // cleanup
    gst_element_set_state(rxPipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(rxPipeline));

    gst_element_set_state(txPipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(txPipeline));
}



void initTxPipeline(GstElement *txPipeline)
{
    GstElement *txSrc, *txSink, *txCsp, *x264enc, *rtph264pay;
/*----------------------------------------------*/ 
//  Create sender pipeline
/*----------------------------------------------*/ 

    if (!(txSrc = gst_element_factory_make("videotestsrc", "txSrc")))
        fprintf(stdout, "txSrc is bogus.");
    if (!(txCsp = gst_element_factory_make("ffmpegcolorspace", "txCsp")))
        fprintf(stdout, "csp is bogus.");
    if (!(x264enc = gst_element_factory_make("x264enc", "x264enc")))
        fprintf(stdout, "x264 is bogus.");
    if (!(rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay")))
        fprintf(stdout, "rtph264pay is bogus.");
    if (!(txSink = gst_element_factory_make("udpsink", "txSink")))
        fprintf(stdout, "Sink is bogus.");

    g_object_set(G_OBJECT(x264enc),"bitrate", 1000, NULL);
    g_object_set(G_OBJECT(x264enc),"byte-stream", TRUE, NULL);
    g_object_set(G_OBJECT(x264enc),"threads", 4, NULL);
    
    g_object_set(G_OBJECT(txSink), "host", "localhost", "port", 5062, NULL);
    

    gst_bin_add_many(GST_BIN(txPipeline), txSrc, txCsp, x264enc, 
            rtph264pay, txSink, NULL);
 
    // links camera first filter and second filter (csp)
    gst_element_link_many(txSrc, txCsp, x264enc, rtph264pay, 
            txSink, NULL);

    // play
    gst_element_set_state(txPipeline, GST_STATE_PLAYING);
}

void initRxPipeline(GstElement *rxPipeline)
{
    GstElement *rxSrc, *ffdec_h264, *rtph264depay, *rxSink;
    GstCaps *caps;

/*----------------------------------------------*/ 
//  Create receiver pipeline
/*----------------------------------------------*/ 

    if (!(rxSrc = gst_element_factory_make("udpsrc", "rxSrc")))
        fprintf(stdout, "Src is bogus.");
    if (!(rtph264depay = gst_element_factory_make("rtph264depay", "rtph264depay")))
        fprintf(stdout, "rtph264depay is bogus.");
    if (!(ffdec_h264= gst_element_factory_make("ffdec_h264", "ffdec_h264")))
        fprintf(stdout, "ffdec_h264 is bogus.");
    if (!(rxSink= gst_element_factory_make("xvimagesink", "rxSink")))
        fprintf(stdout, "rxSink is bogus.");
    

    gst_bin_add_many(GST_BIN(rxPipeline), rxSrc, rtph264depay, 
                        ffdec_h264, rxSink, NULL); 
 
    caps = gst_caps_new_simple("application/x-rtp", NULL);
    if (!caps)
        fprintf(stdout, "caps are bogus.");

    g_object_set(G_OBJECT(rxSrc), "caps", caps, NULL);
    g_object_set(G_OBJECT(rxSrc), "port", 5062, NULL);
    g_object_set(G_OBJECT(rxSink), "sync", FALSE, NULL);

    gst_element_link_many(rxSrc, rtph264depay, ffdec_h264, rxSink, NULL);
    
    // play
    gst_element_set_state(rxPipeline, GST_STATE_PLAYING);
}



void gst_main(int argc, char *argv[])
{
    gst_run();
}

