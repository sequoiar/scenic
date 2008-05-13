// gstApp.cpp

//#include "gst_sip_rtp.h"
#include <iostream>
#include <gst/gst.h>

#include "videoReceiver.h"
#include "videoSender.h"

void eventLoop()
{
    // Approach 1: GMainloop
    //GMainLoop *loop;
    //loop = g_main_loop_new(NULL, FALSE);
    //g_main_loop_run(loop);

    // Approach 2: usleep, combined with a signal handler to set the value
    // of a boolean var named done
    //
    // #include <unistd.h>
    //
    // while(!done)
    // usleep(10000);
    
    // Approach 3: Block waiting for character input
    std::cout << "Hit any key and <cr> to exit." << std::endl << std::endl;
    char c;
    std::cin >> c;
}


void gst_main(int argc, char *argv[])
{
    long port = 5060;

    if (argc > 1)
        port = atoi(argv[1]);

    //    std::cout.flush();
    std::cout << "Using port " << port << std::endl;

    // init gstreamer
    gst_init(0, NULL);  // normally should get argc argv

    // TODO: Figure out why this is way slower when order is reversed
    VideoReceiver rx(port);
    VideoSender tx(port);

    tx.start();
    rx.start();

    /*----------------------------------------------*/ 
    eventLoop();
    /*----------------------------------------------*/ 

    rx.stop();
    tx.stop();
}

