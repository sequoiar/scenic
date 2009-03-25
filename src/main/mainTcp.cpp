/* main.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"
#include "msgThreadFactory.h"

#include "tcp/asioThread.h"

/** Main command line entry point
 * launches the threads and dispatches
 * MapMsg between the threads
*/
class MainModule
    : public BaseModule
{
    public:
        bool run();

        MainModule(bool send, int port)
            : tcpThread_(MsgThreadFactory::Tcp(port, true)),
              gstThread_(MsgThreadFactory::Gst(send)), asio_thread_(new asio_thread()),
              func(gstThread_), msg_count(0){}

        ~MainModule(){delete gstThread_; delete tcpThread_; delete asio_thread_;}
    private:
        MsgThread* tcpThread_;
        MsgThread* gstThread_;
        MsgThread* asio_thread_;
        MainSubscriber func;
        int msg_count;

        MainModule(MainModule&);    //No Copy Constructor
        MainModule& operator=(const MainModule&);
};


bool MainModule::run()
{
    try
    {
        set_handler();
        if(gstThread_ == 0 or !gstThread_->run())
            THROW_ERROR("GstThread not running");
        if(tcpThread_ == 0 or !tcpThread_->run())
            THROW_ERROR("TcpThread not running");
        if(asio_thread_ == 0 or !asio_thread_->run())
            THROW_ERROR("asioThread not running");
        QueuePair &gst_queue = gstThread_->getQueue();
        QueuePair &tcp_queue = tcpThread_->getQueue();
        QueuePair &asio_queue = asio_thread_->getQueue();

        while(!signalFlag())
        {
            MapMsg tmsg = tcp_queue.timed_pop(2000);
            MapMsg gmsg = gst_queue.timed_pop(2000);
            MapMsg amsg = asio_queue.timed_pop(2);

            if(amsg.cmd() == "data")
                LOG_DEBUG(std::string(amsg["str"]));

            if (!gmsg.cmd().empty())
                tcp_queue.push(gmsg);
            if (tmsg.cmd().empty())
                continue;
            LOG_DEBUG(std::string(tmsg.cmd()));
            if (tmsg.cmd() == "quit")
            {
                gstThread_->broadcastQuit();
                break;
            }
            if (tmsg.cmd() == "exception")
                throw tmsg["exception"].except();
            else
            {
                tmsg["id"] = ++msg_count;
                gst_queue.push(tmsg);
                tmsg["ack"] = "ok";
                tcp_queue.push(tmsg);
            }
        }
    }
    catch(ErrorExcept e)
    {
        static int count = 0;
        LOG_WARNING("Abnormal Main Exception:" << e.msg_);
        if (++count > 100)
            throw Except(e);
        return -1;
    }
    LOG_INFO("Normal Program Termination in Progress");
    return 0;
}


void parseArgs(int argc, char** argv)
{
static int port, send;
    if(argc != 3)
        THROW_CRITICAL("Invalid command line arguments -- 0/1 for receive/send and a port");
    if(sscanf(argv[1], "%d", &send) != 1 or send < 0 or send > 1)
        THROW_CRITICAL("Invalid command line arguments -- Send flag must 0 or 1");
    if(sscanf(argv[2], "%d", &port) != 1 or port < 1024 or port > 65000)
        THROW_CRITICAL(
            "Invalid command line arguments -- Port must be in the range of 1024-65000");
}

int telnetServer(int s,int p)
{
    try
    {
        MainModule m(s,p);

        while(m.run())
        {}
    }
    catch(Except e)
    {
        if (e.log_ == ASSERT_FAIL)
            return 1;
    }

    return 0;
}


