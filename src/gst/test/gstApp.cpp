// gstApp.cpp
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

#include "sip/sip.h"

#include "videoReceiver.h"
#include "videoSender.h"
#include "audioSender.h"

#include "defaultAddresses.h"

int eventLoop()
{
}

#if 0
{
    char c;
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
    std::cout << "Hit r and <cr> to request h264.1." << std::endl;
    std::cout << "Hit a and <cr> to accept a request." << std::endl;
    std::cout << "Hit q and <cr> to quit." << std::endl;

    std::cin >> c;

    switch (c)
    {
    case 'q':
        exit(-1);               // FIXME: should rather quit gracefully

    case 'r':
        SipSingleton::Instance()->send_request("h264.1");
        break;

    default:
        break;
    }

    return 0;
}
#endif

void gst_main(int argc, char *argv[])
{
}

#if 0
{
    long txPort = 10010;
    long rxPort = txPort;

    VideoSender tx;
    VideoReceiver rx;

    SipSingleton & sip = *SipSingleton::Instance();

    sip.set_service_port(rxPort);

    if (argc == 5)
        sip.init(argv[1], argv[2], argv[3], argv[4]);
    else
    {
        std::cout << "Using default args." << std::endl;
        sip.init(MY_ADDRESS, "5060", THEIR_ADDRESS, "5061");
    }

    // init gstreamer, moved to MediaBase constructor
    // gst_init(0, NULL);  // normally should get argc argv
    /*----------------------------------------------*/
    for (;;)
    {
        if (sip.handle_events()) {      // if events are queued up
            if ( /*sip.isValidService(sip.get_service()) && */ !tx.isPlaying()) {
                if (tx.init("test", sip.get_service_port(), std::string(MY_ADDRESS))) {
                    tx.start();
                    sip.zero_service_desc();
                }
            }

            if (!rx.isPlaying()) {
                if (sip.get_rx_port()) {
                    if (rx.init(sip.get_rx_port())) {
                        sip.zero_rx_port();
                        rx.start();
                    }
                }
            }
        }

        eventLoop();            // sends requests
    }
}
#endif
