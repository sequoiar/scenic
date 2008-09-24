// Main.cpp
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
 *      Main Module
 */


#include "gstThread.h"
#include "gstSenderThread.h"
#include "gstReceiverThread.h"
#include "tcp/tcpThread.h"
#include "tcp/parser.h"
#include <sstream>
#include "logWriter.h"
#include "lassert.h"


class MainModule
    : public BaseModule
{
    public:
        bool run();

        MainModule(bool send, int port);

        ~MainModule(){delete gstThread_;}
    private:
        TcpThread tcpThread_;
        GstThread* gstThread_;
        MainModule(MainModule&);    //No Copy Constructor
        MainModule& operator=(const MainModule&);
};

MainModule::MainModule(bool send, int port)
        : tcpThread_(port,true), gstThread_(0) 
{
    if(send)
        gstThread_ = new GstSenderThread();
    else
        gstThread_ = new GstReceiverThread();

}

bool MainModule::run()
{
    QueuePair &gst_queue = gstThread_->getQueue();
    QueuePair &tcp_queue = tcpThread_.getQueue();

    if(gstThread_ == 0 || !gstThread_->run())
        LOG_ERROR("GstThread not running");
    if(!tcpThread_.run())
        LOG_ERROR("TcpThread not running");

    while(true)
    {
        MapMsg tmsg = tcp_queue.timed_pop(1000);
        MapMsg gmsg = gst_queue.timed_pop(1);

        if (gmsg["command"].type() != 'n')
            tcpThread_.send(gmsg);
        if (tmsg["command"].type() == 'n')
            continue;
        std::string command;
        if(!tmsg["command"].get(command))
            continue;
        if (command == "quit")
        {
            gst_queue.push(tmsg);
            tcp_queue.push(tmsg);
            break;
        }
        else
            gst_queue.push(tmsg);
    }

    LOG_INFO("Normal Program Termination in Progress");
    return 0;
}

int main (int argc, char** argv)
{
    int port, send;

    try
    {
        if(argc != 3)
            LOG_CRITICAL( "Invalid command line arguments -- 0/1 for receive/send and a port");
        if(sscanf(argv[1], "%d", &send) != 1 || send < 0 || send > 1)
            LOG_CRITICAL("Invalid command line arguments -- Send flag must 0 or 1");
        if(sscanf(argv[2], "%d", &port) != 1 || port < 1024 || port > 65000)
            LOG_CRITICAL(
                "Invalid command line arguments -- Port must be in the range of 1024-65000");
    }
    catch(std::string err)
    {
        std::cerr << "GOING DOWN " << err;
    }
    
    try
    {
        do 
        {
            MainModule m(send, port);

            try{
                if(!m.run())
                    break;
            }
            catch(ErrorExcept e) { std::cerr << e.msg_;}
        } while(1);
    }
    catch(CriticalExcept e) 
    { 
        if (e.log_ == ASSERT_FAIL) 
            std::cerr << "2ND E:" <<e.msg_ <<std::endl;
    }


}

