/* TcpThread.cpp
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

#include "util.h"
#include "tcpThread.h"
#include "parser.h"
#include <errno.h>
#include <string.h>

class TcpLogger
    : public logger::Subscriber
{
    public:
        TcpLogger(TcpThread& tcp)
            : tcp_(tcp){}
        TcpThread& tcp_;
        void operator()(LogLevel&, std::string& msg);
};


void TcpLogger::operator()(LogLevel& level, std::string& msg)
{
    MapMsg m("log");
    m["level"] = level;
    m["msg"] = msg;
    tcp_.send(m);
}

TcpThread::TcpThread(int inport, bool logF)            
: serv_(inport), logFlag_(logF), lf_(new TcpLogger(*this))
{
}

static std::string get_line(std::string& msg)
{
    std::string ret;
    std::string::size_type pos = msg.find_first_of("\n\r");
    if(pos != std::string::npos)
    {
        ret = msg.substr(0, pos+2);
        msg.erase(0, pos+2);
    }
    else{
        ret = msg;
        msg.clear();
    }
    return ret;
}


int TcpThread::main()
{
    bool quit = false;
    std::string msg;

    try
    {
        while(!quit)
        {
            serv_.socket_bind_listen();
            while(!serv_.accept())
            {
                if((quit = gotQuit()))
                    return 0;
                usleep(10000);
            }
            try
            {
                LOG_INFO("Got Connection.");
                if(logFlag_)
                    lf_->enable();
                while(serv_.connected())
                {
                    if((quit = gotQuit()))
                        break;
                    if(serv_.recv(msg))
                    {
                        std::string line = get_line(msg);
                        do
                        {
                            MapMsg mapMsg;
                            if(Parser::tokenize(line, mapMsg))
                                queue_.push(mapMsg);
                            else
                                LOG_WARNING("Bad Msg Received.");
                            line = get_line(msg);
                        }
                        while(!line.empty());
                    }
                    else
                        usleep(1000);
                }
            }
            catch(Except e)
            {
                LOG_DEBUG( "CAUGHT " << e.msg_);
            }
            if(logFlag_)
                lf_->hold();
            if(!quit)
                LOG_WARNING("Disconnected from Core.");
            usleep(1000);
            serv_.close();
        }
    }
    catch(Except e)
    {
        LOG_DEBUG("Passing exception to other Thread" << e.msg_);

        MapMsg mapMsg("exception");
        mapMsg["exception"] = CriticalExcept(e.msg_, e.errno_);
        queue_.push(mapMsg);
    }
    return 0;
}


bool TcpThread::gotQuit()
{
    MapMsg f = queue_.timed_pop(1);
    std::string command;
    if(f["command"].empty())
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
    bool ret;
    lf_->hold(); // to insure no recursive calls due to log message calling send
    try
    {
        Parser::stringify(msg, msg_str);
        ret = serv_.send(msg_str);
        lf_->enable();

    }
    catch(ErrorExcept e)
    {
        LOG_DEBUG(std::string(msg["command"]) << " Error at Send. Cancelled. " <<
                  strerror(e.errno_));
        lf_->enable();
        if(e.errno_ == EBADF) //Bad File Descriptor
            throw (e);
    }

    return ret;
}


bool TcpThread::socket_connect_send(const std::string& addr, MapMsg& msg)
{
    std::string msg_str;
    Parser::stringify(msg, msg_str);
    return serv_.socket_connect_send(addr, msg_str);
}


std::string tcpGetBuffer(int port, int &id)
{
    TcpThread tcp(port);
    tcp.run();
    QueuePair& queue = tcp.getQueue();
    for(;;)
    {
        MapMsg f = queue.timed_pop(100000);
        if(f["command"].empty())
            continue;
        try
        {
            if(std::string(f["command"]) == "quit")
            {
                LOG_INFO("quit in tcpGetBuffer");
                break;
            }
            if(std::string(f["command"]) != "buffer")
            {
                LOG_INFO("Unknown msg.");
                continue;
            }
            id = f["id"];
            return f["str"];
        }
        catch(ErrorExcept)
        {
        }
    }
    return "";
}

#include <errno.h>

bool tcpSendBuffer(const std::string &ip, int port, int id, const std::string &caps)
{
    MapMsg msg("buffer");

    TcpThread tcp(port);

    msg["str"] = caps;
    msg["id"] = id;

    const int MAX_TRIES = 100;

    for(int i = 0; i < MAX_TRIES; ++i)
    {
        try
        {
            bool ret = tcp.socket_connect_send(ip, msg);
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


