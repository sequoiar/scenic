

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
#include "osc/osc.h"
#include "gutil/optionArgs.h"
#include <sstream>

class MainModule
    : public BaseModule
{
    public:
        bool run();
        MainModule();
    private:
        GstThread gstThread_;
        OscThread oscThread_;

    int port_;
};


MainModule::MainModule()
    :BaseModule(),gstThread_(),oscThread_(),port_(0)
{

    args_.push_back(new IntArg(&port_, "oscPort", 'p', "Set the osc incomming port","port num"));


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
//    QueuePair &gst_queue = gstThread_.getQueue();
    OscQueue &osc_queue = oscThread_.getQueue();

//    if(!gstThread_.run())
//        return 0;
    std::stringstream s;
    s << port_;
    oscThread_.set_local_port(s.str());
    if(!oscThread_.run())
        return 0;

    while(1)
    {
        OscMessage m = osc_queue.timed_pop(10000);

        if(m.pathIsSet())
        {
            m.print();
            continue;
        }
        if(m.pathEquals("/quit"))
        {
//            BaseMessage in(BaseMessage::QUIT);
//            gst_queue.push(in);
            osc_queue.push(OscMessage("/quit", "", 0, 0, 0));
            break;
        }
        if(m.pathEquals("/gst"))
            continue;
        //LOG(m.args[0].s);
/*        if(!m.args[0].s.compare("init")){
            BaseMessage in(BaseMessage::INIT);
            gst_queue.push(in);
        }
        if(!m.args[0].s.compare("start")){
            BaseMessage start(BaseMessage::START);
            gst_queue.push(start);
        }
        if(!m.args[0].s.compare("stop")){
            BaseMessage stop(BaseMessage::STOP);
            gst_queue.push(stop);
        }
*/
    }

    std::cout << "Done!" << std::endl;
    return 0;
}




//./mainTester -s videotestsrc --oscLocal=7770 --oscRemote=7771 --oscRemoteHost=127.0.0.1
