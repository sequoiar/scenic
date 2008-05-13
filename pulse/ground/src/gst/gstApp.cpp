// gstApp.cpp

//#include "gst_sip_rtp.h"
#include <iostream>
#include <gst/gst.h>

#include "videoReceiver.h"
#include "videoSender.h"

void gst_main(int argc, char *argv[])
{
    // create gstreamer components
    GMainLoop *loop;
    long port = 5060;

    if (argc > 1)
        port = atoi(argv[1]);

    std::cout.flush();
    std::cout << "Using port " << port << std::endl;

    // init gstreamer
    gst_init(0, NULL);  // normally should get argc argv
    loop = g_main_loop_new(NULL, FALSE);

    // TODO: Figure out why this is way slower when order is reversed
    VideoReceiver rx(port);
    VideoSender tx(port);

    rx.start();
    tx.start();
    
    // SPIN HERE
    /*----------------------------------------------*/ 
    g_main_loop_run(loop);
    /*----------------------------------------------*/ 

    rx.stop();
    tx.stop();
}

