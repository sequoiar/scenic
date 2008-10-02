/* GTHREAD-QUEUE-PAIR - Library of TcpThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redisttribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <iostream>
#include "tcpThread.h"
#include "logWriter.h"
#include "parser.h"

class TcpLogFunctor : public LogFunctor
{
public:
    TcpLogFunctor(TcpThread& tcp):tcp_(tcp){}
    TcpThread& tcp_;
    void operator()(LogLevel&,std::string& msg);
};

void TcpLogFunctor::operator()(LogLevel& level,std::string& msg)
{
    MapMsg m("log");
    m["level"] = level;
    m["msg"] = msg;
    tcp_.send(m);
}

int TcpThread::main()
{
    bool quit = false;
    std::string msg;
    TcpLogFunctor lf_(*this);

    try
    {
    while(!quit)
    {
        serv_.socket_bind_listen();
        try
        {
            while(!serv_.accept())
            {
                if((quit = gotQuit()))
                    return 0;
                usleep(10000);
            }
            LOG_INFO("Got Connection.");
            if(logFlag_)
                LOG::register_cb(&lf_);
            while(serv_.connected())
            {
                if((quit = gotQuit()))
                {
                    break;
                }
                if(serv_.recv(msg))
                {
                    MapMsg mapMsg;
                    if(Parser::tokenize(msg, mapMsg))
                        queue_.push(mapMsg);
                    else
                        LOG_WARNING("Bad Msg Received.");
                }
                else
                    usleep(1000);
            }
            if(logFlag_)
                LOG::unregister_cb();
            if(!quit)
                LOG_WARNING("Disconnected from Core.");
            usleep(1000);
            serv_.close();
        }
        catch(Except e)
        {
            LOG_DEBUG( "CAUGHT " << e.msg_);
        }
    }
    }
    catch(Except e)
    {
        LOG_DEBUG("Passing exception to other Thread" << e.msg_);

        MapMsg mapMsg("exception");
        mapMsg["exception"] = CriticalExcept(e.msg_,e.errno_);
        queue_.push(mapMsg);
    }
    return 0;
}


bool TcpThread::gotQuit()
{
    MapMsg f = queue_.timed_pop(1);
    std::string command;
    if(f["command"].type() == 'n')
        return false;

    if(f["command"].get(command)&& command == "quit")
        return true;
    else
        send(f);

    return false;
}


bool TcpThread::send(MapMsg& msg)
{
    std::string msg_str;
    LOG::hold_cb(); // to insure no recursive calls due to log message calling send 
    Parser::stringify(msg, msg_str);
    bool ret = serv_.send(msg_str);
    LOG::release_cb();
    return ret;
}

bool TcpThread::socket_connect_send(const std::string& addr, MapMsg& msg)
{
    std::string msg_str;
    Parser::stringify(msg, msg_str);
    LOG_DEBUG(msg_str);
    return serv_.socket_connect_send(addr,msg_str);
}

