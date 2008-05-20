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
    std::cout << "Hit r and <cr> to send a request." << std::endl << std::endl;
    char c;
    std::cin >> c;

    switch(c)
    {
        case 'q':
            exit(-1);       // FIXME: should rather quit gracefully
        
        case 'r':
            SipSingleton::Instance()->send_request("h264.1");
            break;
    }

    return 0;
}


void gst_main(int argc, char *argv[])
{
    long txPort = 10060;
    long rxPort = txPort;
   
    VideoSender tx;
    VideoReceiver rx;
#if 0
    if (argc > 2)
    {
        txPort = atoi(argv[1]);
        rxPort = atoi(argv[2]);
    }
    else
        std::cout << "Usage: " << std::endl << 
                     "gst <sendToPort> <listenToPort>" 
                     << std::endl << std::endl;
#endif
    
    SipSingleton &sip = *SipSingleton::Instance();
    
    sip.set_service_port(rxPort);

    if (argc == 5)
        sip.init(argv[1], argv[2], argv[3],argv[4]);
    else
        sip.init(MY_ADDRESS, "5060", THEIR_ADDRESS, "5061");

    // init gstreamer
    gst_init(0, NULL);  // normally should get argc argv
//    sip.send_request("h264.1");
    /*----------------------------------------------*/ 
    for(;;)
    {
        std::cout << "inloop" << std::endl;
        if(sip.handle_events())
        {
            if(!strcmp(sip.get_service(),"h264.1"))
            {
                
                tx.init(sip.get_service_port());
                tx.start();
                sip.zero_service_desc();
            }

            if(sip.get_rx_port())
            {
                std::cout << "!!!!!!!!" << sip.get_rx_port() << std::endl;
                rx.init(sip.get_rx_port());
                sip.zero_rx_port();
                rx.start();
            }
        }

        eventLoop();
    }


#if 0
    /*----------------------------------------------*/ 
    //    std::cout.flush();
    std::cout << "Sending to port " << txPort << std::endl;
    std::cout << "Listening to port " << rxPort << std::endl;

    VideoSender tx(txPort);
    VideoReceiver rx(rxPort);

    tx.start();
    rx.start();


    rx.stop();
    tx.stop();
#endif
}

#if 0
int gst_main(int argc, char *argv[])
{
    SipSingleton &sip = *SipSingleton::Instance();

    sip.set_service_port(10010);

    if(!sip.init(argc,argv))
        return -1;

    if (argc == 5)
    {
        //        sip.send_request("h264.1");
    }


    for (;;)
    {
        static int eventCount;
        std::cout << "here" << std::endl;
        if (eventCount += sip.handle_events())
        {
            std::cout << "HANDLED " << eventCount << " EVENTS " << std::endl;
        }
    }

    return 0;
}
#endif
