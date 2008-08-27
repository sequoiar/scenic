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
#include "gutil/optionArgs.h"
#include <sstream>
#include "logWriter.h"

class MainModule
    : public BaseModule
{
    public:
        bool run();

        MainModule();
    private:
        GstThread gstThread_;
        TcpThread tcpThread_;

        int port_;
};


MainModule::MainModule()
    : BaseModule(), gstThread_(), tcpThread_(10000), port_(0)
{
//args_.push_back(new IntArg(&port_, "tcpPort", 'p', "Set the tcp incomming port","port num"));
}


int main (int , char** )
{
    MainModule m;
    //OptionArgs opts;

//    opts.add(m.get_args());

//    if(!opts.parse(argc, argv))
//       return 1;
    m.run();
    return 0;
}


bool MainModule::run()
{
    QueuePair &gst_queue = gstThread_.getQueue();
    TcpQueue &tcp_queue = tcpThread_.getQueue();

    if(!gstThread_.run())
        return 0;
    std::stringstream s;
    s << port_;

    //tcpThread_.set_local_port(s.str());

    if(!tcpThread_.run())
        return 0;
    while(true)
    {
        TcpMessage m = tcp_queue.timed_pop(10000);
        std::string& mstr = m.getMsg();
        GstMsg gmsg = gst_queue.timed_pop(1000);
        if (!gmsg.getMsg().empty())
            LOG_DEBUG(gmsg.getMsg());

        if (mstr.empty())
            continue;

        mstr.erase(mstr.size()-2,2);

        std::cout << m.getMsg().size();

        if (!m.getMsg().compare("quit"))
        {
            GstMsg in(GstMsg::QUIT);
            gst_queue.push(in);
            LOG("in quit!", DEBUG);
            tcp_queue.push(TcpMessage(TcpMessage::QUIT));
            break;
        }
        else if (!m.getMsg().compare("init")) {
            GstMsg in(GstMsg::INIT);
            gst_queue.push(in);
        }
        else if (!m.getMsg().compare("start")) {
            GstMsg start(GstMsg::START);
            gst_queue.push(start);
          
        }
        else if (!m.getMsg().compare("stop")) {
            GstMsg stop(GstMsg::STOP);
            gst_queue.push(stop);
        }
        else
            LOG_DEBUG("Unknown command");
    }

    std::cout << "Done!" << std::endl;
    return 0;
}


//./mainTester -s videotestsrc --tcpLocal=7770 --tcpRemote=7771 --tcpRemoteHost=127.0.0.1
