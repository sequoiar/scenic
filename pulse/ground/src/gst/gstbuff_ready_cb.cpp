// gstbuff_ready_cb.c

//#include "gst_sip_rtp.h"
#include <gst/gst.h>

// FIXME: these shouldn't be global

void gst_run()
{
    // create gstreamer components
    GstElement *txPipeline, *txSrc, *txSink, *txCsp, 
               *txFlt, *x264enc, *rtph264pay;
    GstElement *rxPipeline, *rxSrc, *ffdec_h264, *rtph264depay, *rxSink;
    GstCaps *caps;
    GMainLoop *loop;
    
    // init gstreamer
    gst_init(0, NULL);  // normally should get argc argv
    loop = g_main_loop_new(NULL, FALSE);

/*----------------------------------------------*/ 
//  Create sender pipeline
/*----------------------------------------------*/ 

    if (!(txPipeline = gst_pipeline_new("txPipeline")))
        fprintf(stdout, "Pipeline is bogus.");
    if (!(txSrc = gst_element_factory_make("videotestsrc", "txSrc")))
        fprintf(stdout, "txSrc is bogus.");
    //if (!(txFlt = gst_element_factory_make("capsfilter", "txFlt")))
     //   fprintf(stdout, "FLT is bogus.");
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

#if 0
    // pad refers to input of sink element
    txPad = gst_element_get_pad(GST_ELEMENT(txSink), "sink");

    // add probe to sink's input
    gst_pad_add_buffer_probe(txPad, G_CALLBACK(fakesink_handoff), NULL);
    gst_object_unref(txPad); 
#endif 
    
/*----------------------------------------------*/ 
//  Create receiver pipeline
/*----------------------------------------------*/ 

    if (!(rxPipeline = gst_pipeline_new("rxPipeline")))
        fprintf(stdout, "rxPipeline is bogus.");
    if (!(rxSrc = gst_element_factory_make("udpsrc", "rxSrc")))
        fprintf(stdout, "Src is bogus.");
    if (!(rtph264depay = gst_element_factory_make("rtph264depay","rtph264depay")))
        fprintf(stdout, "rtph264depay is bogus.");
    if (!(ffdec_h264= gst_element_factory_make("ffdec_h264", "ffdec_h264")))
        fprintf(stdout, "ffdec_h264 is bogus.");
    if (!(rxSink= gst_element_factory_make("xvimagesink", "rxSink")))
        fprintf(stdout, "rxSink is bogus.");
    

    gst_bin_add_many(GST_BIN(rxPipeline), rxSrc, rtph264depay, ffdec_h264, rxSink, NULL); 
 
    caps = gst_caps_new_simple("application/x-rtp", NULL);
    if (!caps)
        fprintf(stdout, "caps are bogus.");

    g_object_set(G_OBJECT(rxSrc), "caps", caps, NULL);

    g_object_set(G_OBJECT(rxSrc), "port", 5062, NULL);
    g_object_set(G_OBJECT(rxSink), "sync", FALSE, NULL);

    gst_element_link_many(rxSrc, rtph264depay, ffdec_h264, rxSink, NULL);
    
#if 0
    // set buffer size to 1400
    g_object_set(G_OBJECT(rxSrc), "signal-handoffs", TRUE, 
                    "sizemax", 1400, "sizetype", 2, NULL);
    g_signal_connect(rxSrc, "handoff", G_CALLBACK(fakesrc_handoff), NULL);
#endif

    // play
    gst_element_set_state(txPipeline, GST_STATE_PLAYING);
    gst_element_set_state(rxPipeline, GST_STATE_PLAYING);

    /*----------------------------------------------*/ 
    g_main_loop_run(loop);
    /*----------------------------------------------*/ 

    // cleanup
    gst_element_set_state(rxPipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(rxPipeline));

    gst_element_set_state(txPipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(txPipeline));
}




void gst_main(int argc, char *argv[])
{
    gst_run();

}


















