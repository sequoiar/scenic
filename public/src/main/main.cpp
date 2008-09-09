// headerGPL.c
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

/** \file
 *      Just the License GPL 3+
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */


#include "gstThread.h"
#include "tcp/tcpThread.h"
#include "tcp/parser.h"
#include <sstream>
#include "logWriter.h"

class MainModule
    : public BaseModule
{
    public:
        bool run();

        MainModule(int port)
            : BaseModule(), gstThread_(), tcpThread_(port){}
    private:
        GstThread gstThread_;
        TcpThread tcpThread_;
};


int main (int argc, char** argv)
{
    int port;
    if(argc != 2)
    {
        LOG_INFO("Must provide a port");
        return -1;
    }
    if(sscanf(argv[1], "%d", &port) != 1 || port < 0 || port > 65000)
    {
        LOG_INFO("Port must be in the range of 1-65000");
        return -1;
    }
    MainModule m(port);
    m.run();
    return 0;
}


bool MainModule::run()
{
    QueuePair &gst_queue = gstThread_.getQueue();
    TcpQueue &tcp_queue = tcpThread_.getQueue();

    if(!gstThread_.run())
        return 0;
    if(!tcpThread_.run())
        return 0;
    while(true)
    {
        MapMsg tmsg = tcp_queue.timed_pop(10000);
        MapMsg gmsg = gst_queue.timed_pop(1000);

        if (gmsg["command"].type() != 'n')
        {
            std::string tstr;
            gmsg["command"].get(tstr);
            LOG_DEBUG(tstr);
            if(!tstr.compare("caps"))
            {
                std::string caps_str;
                if(gmsg["caps_str"].get(caps_str))
                {
                    LOG_DEBUG(caps_str);
                    tcpThread_.send(gmsg);
                }
            }
        }
        if (tmsg["command"].type() == 'n')
            continue;

        std::string command;
        if(!tmsg["command"].get(command))
            continue;

        if (!command.compare("quit"))
        {
            gst_queue.push(tmsg);
            LOG("in quit!", DEBUG);
            tcp_queue.push(tmsg);
            break;
        }
        else
            gst_queue.push(tmsg);
    }

    std::cout << "Done!" << std::endl;
    return 0;
}


