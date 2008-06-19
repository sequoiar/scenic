// sipMain.cpp
// Copyright 2008 Koya Charles & Tristan Matthews 
//     
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//


#include <iostream>
#include <cassert>
#include <gst/gst.h>

#include "sip.h"

#include "gst/videoReceiver.h"
#include "gst/videoSender.h"
#include "gst/audioSender.h"

#include "defaultAddresses.h"

int main(int argc, char *argv[])
{
    SipSingleton &sip = *SipSingleton::Instance();
    return 0;
}
#if 0
bool eventLoop()
{
    char c[2];
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
    std::cout << "Hit r and <cr> to request H264" << std::endl;
//    std::cout << "Hit a and <cr> to accept a request." << std::endl; 
    std::cout << "Hit q and <cr> to quit." << std::endl; 

    std::cin.getline(c,2);


    switch(c[0])
    {
        case 'q':
            return 0;
            break;

        case 'r':
            {
                Sdp sdp("sipT");
                SdpMedia sdpm = SdpMediaFactory::clone("H264");
                sdpm.set_ip(MY_ADDRESS);
                sdpm.set_port(SipSingleton::Instance()->get_service_port());
                sdp.add_media(sdpm);
                SipSingleton::Instance()->send_request(sdp.str());
            }
            break;

        default:
            break;
    }

    return 1;
}


int main(int argc, char *argv[])
{
    long txPort = 11001;
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

    // init gstreamer, moved to MediaBase constructor
    // gst_init(0, NULL);  // normally should get argc argv
    /*----------------------------------------------*/ 
    while(eventLoop())        // sends requests
    {
        if(sip.handle_events()) // if events are queued up
        {
            Sdp& sdp_t = sip.get_sdp();
            if(sdp_t.is_valid() && !tx.isPlaying())
            {
                for(SdpMediaIterator it=sdp_t.get_media_begin();it != sdp_t.get_media_end();it++)
                {
                    std::cout << it->get_media_type() << " MEDIA TYPE" << std::endl;
                    if(!it->get_media_type().compare("video"))
                        if(tx.init("test",it->get_port(), it->get_ip()))
                        {
                            tx.start();
                            sip.zero_service_desc();
                        }
                }
            }


            if (!rx.isPlaying())
            {
                if(sip.response_ok())
                {
                    if(rx.init(sip.get_service_port()))
                    {
                        sip.zero_service_port();
                        rx.start();
                    }
                }
            }
        }

    }
    return 0;
}
#endif

