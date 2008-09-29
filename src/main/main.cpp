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

#include "builder.h"
#include "logWriter.h"


class MainModule
    : public BaseModule
{
    public:
        bool run();

        MainModule(bool send, int port)
            : tcpThread_(Builder::TcpBuilder(port,true)), 
              gstThread_(Builder::GstBuilder(send)) {}

        ~MainModule(){delete gstThread_; delete tcpThread_;}
    private:
        MsgThread* tcpThread_;
        MsgThread* gstThread_;
        MainModule(MainModule&);    //No Copy Constructor
        MainModule& operator=(const MainModule&);
};


bool MainModule::run()
{

    try
    {
        if(gstThread_ == 0 || !gstThread_->run())
            THROW_ERROR("GstThread not running");
        if(tcpThread_ == 0 || !tcpThread_->run())
            THROW_ERROR("TcpThread not running");
        QueuePair &gst_queue = gstThread_->getQueue();
        QueuePair &tcp_queue = tcpThread_->getQueue();
        while(true)
        {
            MapMsg tmsg = tcp_queue.timed_pop(1);
            MapMsg gmsg = gst_queue.timed_pop(1000);

            if (gmsg["command"].type() != 'n')
                tcp_queue.push(gmsg);
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
            if (command == "exception")
            {       
                Except e;
                tmsg["exception"].get(e);
                throw e;
            }
            else
                gst_queue.push(tmsg);
        }
    }
    catch(Except e)
    {
        static int count = 0; 
        LOG_INFO("Abnormal Main Exception:" << e.msg_);
        if (++count > 100)
            throw Except(e);
        return -1;
    }
    LOG_INFO("Normal Program Termination in Progress");
    return 0;
}

static int port, send;
void parseArgs(int argc,char** argv)
{
    if(argc != 3)
        THROW_CRITICAL("Invalid command line arguments -- 0/1 for receive/send and a port");
    if(sscanf(argv[1], "%d", &send) != 1 || send < 0 || send > 1)
        THROW_CRITICAL("Invalid command line arguments -- Send flag must 0 or 1");
    if(sscanf(argv[2], "%d", &port) != 1 || port < 1024 || port > 65000)
{}//        THROW_CRITICAL("Invalid command line arguments -- Port must be in the range of 1024-65000");
}

int main (int argc, char** argv)
{

    try
    {
        parseArgs(argc,argv);    
        MainModule m(send, port);

        while(m.run()){}
    }
    catch(Except e) 
    { 
        if (e.log_ == ASSERT_FAIL) 
            return 1;
    }

    return 0;
}

