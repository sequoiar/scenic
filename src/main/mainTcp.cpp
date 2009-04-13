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

class Logger
    : public Log::Subscriber
{
    public:
        Logger(MsgThread& tcp)
            : queue_(tcp.getQueue()){}
        QueuePair& queue_;
        void operator()(LogLevel&, std::string& msg);
};


void Logger::operator()(LogLevel& level, std::string& msg)
{
    //MapMsg m("log");
    //m["level"] = level;
    //m["msg"] = msg;
    //queue_.push(m);
    std::cout << level << " "<< msg;
}

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
              gstThread_(MsgThreadFactory::Gst(send)),
              func(gstThread_), msg_count(0)
        {}

        ~MainModule()
        {
            delete gstThread_; 
            delete tcpThread_; 
        }

    private:
        MsgThread* tcpThread_;
        MsgThread* gstThread_;
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
        Logger logger_(*tcpThread_);
//        if(gstThread_ == 0 or !gstThread_->run())
//            THROW_ERROR("GstThread not running");
        if(tcpThread_ == 0 or !tcpThread_->run())
            THROW_ERROR("TcpThread not running");
        //QueuePair &gst_queue = gstThread_->getQueue();
        QueuePair &tcp_queue = tcpThread_->getQueue();

        while(!signalFlag())
        {
            LOG_DEBUG("MAINLOOP");
            MapMsg tmsg = tcp_queue.timed_pop(2000000);
            //MapMsg gmsg = gst_queue.timed_pop(2000);
            //MapMsg amsg = asio_queue.timed_pop(2);

            //if(amsg.cmd() == "data")
            //    LOG_DEBUG(std::string(amsg["str"]));

            //if (!gmsg.cmd().empty())
            //    tcp_queue.push(gmsg);
            if (tmsg.cmd().empty())
                continue;
            LOG_DEBUG(std::string(tmsg.cmd()));
            if (tmsg.cmd() == "quit")
            {
                MsgThread::broadcastQuit();
                break;
            }
            if (tmsg.cmd() == "exception")
                throw tmsg["exception"].except();
            else
            {
                tmsg["id"] = ++msg_count;
              //  gst_queue.push(tmsg);
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
