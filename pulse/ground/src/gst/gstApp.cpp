// gstApp.cpp

//#include "gst_sip_rtp.h"
#include <iostream>
#include <gst/gst.h>

#include "../sip/sipSingleton.h"

#include "videoReceiver.h"
#include "videoSender.h"





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
    std::cout << "Hit any key and <cr> to exit." << std::endl << std::endl;
    char c;
    std::cin >> c;
    if (c == 'q')
        return -1;
    if (c == 'r')
        return 1;

    return 0;
}


void gst_main(int argc, char *argv[])
{
    long txPort = 15060;
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

    sip.init("192.168.1.164","5061","192.168.1.164","5060");

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
                tx.init(sip.get_rx_port());
                tx.start();
                sip.zero_rx_port();
            }
            if(sip.get_rx_port())
            {
                rx.init(sip.get_tx_port());
                rx.start();
            }
        }
        switch (eventLoop())
        {
            case 0: break;
            case 1: sip.send_request("h264.1");break;
            case -1: exit(-1);

            default:;
        }
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
