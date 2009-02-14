/* main.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

///Instance will register a particular MsgThread as a MapMsg handler
class MainSubscriber
    : public msg::Subscriber
{
    MsgThread &t_;
    public:
        MainSubscriber(MsgThread* pt)
            : t_(*pt)
        {}

        void operator()(MapMsg& msg)
        {
            t_.getQueue().push(msg);
        }
};

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
              func(gstThread_), msg_count(0){}

        ~MainModule(){delete gstThread_; delete tcpThread_;}
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
        if(gstThread_ == 0 || !gstThread_->run())
            THROW_ERROR("GstThread not running");
        if(tcpThread_ == 0 || !tcpThread_->run())
            THROW_ERROR("TcpThread not running");
        QueuePair &gst_queue = gstThread_->getQueue();
        QueuePair &tcp_queue = tcpThread_->getQueue();

        while(!signalFlag())
        {
            MapMsg tmsg = tcp_queue.timed_pop(1);
            MapMsg gmsg = gst_queue.timed_pop(1000);

            if (!gmsg["command"].empty())
                tcp_queue.push(gmsg);
            if (tmsg["command"].empty())
                continue;
            std::string command;
            if(!tmsg["command"].get(command))
                continue;
            LOG_DEBUG(std::string(tmsg["command"]));
            if (command == "quit")
                break;
            if (command == "exception")
                throw tmsg["exception"].except();
            else
            {
                gst_queue.push(tmsg);
                tmsg["ack"] = "ok";
                tmsg["id"] = ++msg_count;
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


static int port, send;
void parseArgs(int argc, char** argv)
{
    if(argc != 3)
        THROW_CRITICAL("Invalid command line arguments -- 0/1 for receive/send and a port");
    if(sscanf(argv[1], "%d", &send) != 1 || send < 0 || send > 1)
        THROW_CRITICAL("Invalid command line arguments -- Send flag must 0 or 1");
    if(sscanf(argv[2], "%d", &port) != 1 || port < 1024 || port > 65000)
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


