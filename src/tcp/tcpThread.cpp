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
    void cb(LogLevel&,std::string& msg);
};

void TcpLogFunctor::cb(LogLevel& level,std::string& msg)
{
    MapMsg m;
    m["command"] = StrIntFloat("log");
    m["level"] = StrIntFloat(level);
    m["msg"] = StrIntFloat(msg);
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
            if(!serv_.socket_bind_listen())
                return -1;

            while(!serv_.accept())
                usleep(10000);

            LOG_INFO("Got Connection.");
            register_cb(&lf_);
            while(serv_.connected())
            {
                if((quit = gotQuit()))
                {
                    break;
                }
                if(serv_.recv(msg))
                {
                    MapMsg mapMsg;
                    if(tokenize(msg, mapMsg))
                        queue_.push(mapMsg);
                    else
                        LOG_WARNING("Bad Msg Received.");
                }
                else
                    usleep(10000);
            }
            release_cb();
            if(!quit)
                LOG_WARNING("Disconnected from Core.");

            serv_.close();
        }
    }
    catch(Except e)
    {
        std::cerr << e.msg_;
    }
    return 0;
}


bool TcpThread::gotQuit()
{
    MapMsg f = queue_.timed_pop(1);
    std::string command;
    if(f["command"].get(command)&& !command.compare("quit"))
    {
        queue_.push(f);
        return true;
    }
    return false;
}


bool TcpThread::send(MapMsg& msg)
{
    std::string msg_str;
    stringify(msg, msg_str);
    return serv_.send(msg_str);
}

bool TcpThread::socket_connect_send(const std::string& addr, const MapMsg& msg)
{
    std::string msg_str;
    stringify(msg, msg_str);
    LOG_DEBUG(msg_str);
    return serv_.socket_connect_send(addr,msg_str);

}

