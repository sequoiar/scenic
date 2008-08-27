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
//#include "logWriter.h"

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
    args_.push_back(new IntArg(&port_, "tcpPort", 'p', "Set the tcp incomming port",
                               "port num"));
}


int main (int argc, char** argv)
{
    MainModule m;
    OptionArgs opts;

    opts.add(m.get_args());

    if(!opts.parse(argc, argv))
        return 1;
    m.run();
    return 0;
}


bool MainModule::run()
{
#if 0 
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

        if (m.getMsg().empty())
            continue;
        LOG_DEBUG(m.getMsg());

        if (m.pathEquals("/quit"))
        {
            GstMsg in(GstMsg::QUIT);
            gst_queue.push(in);
            LOG("in quit!", DEBUG);
            tcp_queue.push(TcpMessage("/quit", "", 0, 0, 0));
            break;
        }
        else if (!m.pathEquals("/gst"))
            continue;
        if (m.argEquals("init", 0)) {
            GstMsg in(GstMsg::INIT);
            gst_queue.push(in);
        }
        else if (m.argEquals("start", 0)) {
            GstMsg start(GstMsg::START);
            gst_queue.push(start);
        }
        else if (m.argEquals("stop", 0)) {
            GstMsg stop(GstMsg::STOP);
            gst_queue.push(stop);
        }
    }

    std::cout << "Done!" << std::endl;
#endif
    return 0;
}


//./mainTester -s videotestsrc --tcpLocal=7770 --tcpRemote=7771 --tcpRemoteHost=127.0.0.1
