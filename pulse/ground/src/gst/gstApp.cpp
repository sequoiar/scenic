// gstApp.cpp

//#include "gst_sip_rtp.h"
#include <iostream>
#include <cassert>
#include <gst/gst.h>

#include "../sip/sipSingleton.h"

#include "videoReceiver.h"
#include "videoSender.h"

#include "defaultAddresses.h"


int eventLoop()
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
    //bool done = false;
    //while(!done)
    //usleep(10000);
    
    // Approach 3: Block waiting for character input
    std::cout << "Hit t and <cr> to send testfile." << std::endl;
    std::cout << "Hit d and <cr> to send dv." << std::endl;
    std::cout << "Hit a and <cr> to accept a request." << std::endl; 
    std::cout << "Hit q and <cr> to quit." << std::endl; 

    char c;
    std::cin >> c;

    switch(c)
    {
        case 'q':
            exit(-1);       // FIXME: should rather quit gracefully
        
        case 'r':
            SipSingleton::Instance()->send_request("h264.1");
            break;

        case 'd':
            SipSingleton::Instance()->send_request("dv");
            break;

        case 't':
            SipSingleton::Instance()->send_request("test");
            break;

        default:
            break;
    }

    return 0;
}


void gst_main(int argc, char *argv[])
{
    long txPort = 10010;
    long rxPort = txPort;
   
    VideoSender tx;
    VideoReceiver rx;
    
    SipSingleton &sip = *SipSingleton::Instance();
    
    sip.set_service_port(rxPort);

    if (argc == 5)
        sip.init(argv[1], argv[2], argv[3],argv[4]);
    else
    {
        std::cout << "Using default args." << std::endl;
        sip.init(MY_ADDRESS, "5060", THEIR_ADDRESS, "5061");
    }

    // init gstreamer
    gst_init(0, NULL);  // normally should get argc argv
    /*----------------------------------------------*/ 
    for(;;)
    {
        if(sip.handle_events()) // if events are queued up
        {
            if(sip.isValidService(sip.get_service()) && !tx.isPlaying())
            {
                if(tx.init(sip.get_service_port(), std::string(MY_ADDRESS), 
                            sip.get_service()))
                {
                    tx.start();
                    sip.zero_service_desc();
                }
            }


            if (!rx.isPlaying())
            {
                if(sip.get_rx_port())
                {
                    if(rx.init(sip.get_rx_port()))
                    {
                        sip.zero_rx_port();
                        rx.start();
                    }
                }
            }
        }

        eventLoop();        // sends requests
    }
}

