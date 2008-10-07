// capsHelper.h
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

#include "tcp/tcpThread.h"
#include "tcp/parser.h"
#if 0
//template<class T>
StrIntFloat p(MapMsg& in,std::string key,std::string file, std::string line)
{
    try
    {
        return in[key];
    }
    catch(Except e)
    {
        e.msg_.append(file);
        e.msg_.append(line);
        throw(e);
    }
    return StrIntFloat();
}

#define F(f,x) p(f,x,__FILE__,"__LINE__")
#endif
/*----------------------------------------------*/ 
/* Helper functions                             */
/*----------------------------------------------*/ 

#if 0
static bool tcpGetCaps(int port, AudioReceiver &rx)
{
    TcpThread tcp(port);
    tcp.run();
    QueuePair& queue = tcp.getQueue();
    for(;;)
    {
        MapMsg f = queue.timed_pop(100000);
        if(f["command"].type() == 'n')
            continue;
        try
        {
            rx.set_caps(f["caps_str"]);
            break;
        }
        catch(ErrorExcept)
        {
            return false;
        }
    }
    return true;
}
#endif


static std::string tcpGetCaps(int port)
{
    TcpThread tcp(port);
    tcp.run();
    QueuePair& queue = tcp.getQueue();
    for(;;)
    {
        MapMsg f = queue.timed_pop(100000);
        if(f["command"].type() == 'n')
            continue;
        try
        {
            return (std::string(f["caps_str"]));
        }
        catch(ErrorExcept)
        {
        }
    }
    return "";
}

#include <errno.h>

static bool tcpSendCaps(int port, const std::string &caps)
{
    MapMsg msg;
    std::ostringstream s;

    TcpThread tcp(port);
    s  << "caps: caps_str=\"" << Parser::strEsq(caps) <<"\"" << std::endl;
    Parser::tokenize(s.str(),msg);

    const int MAX_TRIES = 100;

    for(int i = 0; i < MAX_TRIES; ++i)
    {
        try
        {
            bool ret = tcp.socket_connect_send("127.0.0.1", msg);
            if(ret)
                return true;
        }
        catch(ErrorExcept e)
        {
           if(e.errno_ == ECONNREFUSED ) 
               LOG_DEBUG("GOT ECONNREFUSED");
           else
               return false;
        }
        usleep(100000);
    }
    return false;
}


