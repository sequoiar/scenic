/* TcpThread.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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
#include <errno.h>
#include <string.h>

class TcpLog
   // : public Log::Subscriber
{
    public:
        TcpLog(){}
        void operator()(LogLevel&, std::string&){}
        void hold(){}
        void enable(){}
};

std::auto_ptr<TcpLog> lf_(new TcpLog());

TcpThread::TcpThread(int inport, bool logF)            
: serv_(inport), logFlag_(logF)
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


void TcpThread::main()
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
                    break;
            }
            if(!quit)
            {
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
                                if(mapMsg.tokenize(line))
                                    queue_.push(mapMsg);
                                else
                                    LOG_WARNING("Bad Msg Received.");
                                line = get_line(msg);
                            }
                            while(!line.empty());
                        }
                    }
                }
                catch(Except e)
                {
                    LOG_DEBUG( "CAUGHT " << e.msg_);
                }
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
}


bool TcpThread::gotQuit()
{
    MapMsg f = queue_.timed_pop(2000);
    std::string command;
    if(f.cmd().empty())
        return false;
    if(f.cmd() == "quit")
        return true;
    else
        send(f);
    return false;
}


bool TcpThread::send(MapMsg& msg)
{
    std::string msg_str;
    bool ret= false;
    if(th_)
    {
        try
        {
            lf_->hold(); // to insure no recursive calls due to log message calling send
            msg.stringify(msg_str);
            ret = serv_.send(msg_str);
            lf_->enable();

        }
        catch(ErrorExcept e)
        {
            LOG_DEBUG(msg.cmd() << " Error at Send. Cancelled. " << strerror(e.errno_));
            lf_->enable();
            //if(e.errno_ == EBADF) //Bad File Descriptor
            //    throw (e);
        }
    }
    return ret;
}


bool TcpThread::socket_connect_send(const std::string& addr, MapMsg& msg)
{
    std::string msg_str;
    msg.stringify(msg_str);
    return serv_.socket_connect_send(addr, msg_str);
}


std::string tcpGetBuffer(int port, int &id)
{
    std::string ret;
    TcpThread tcp(port);
    tcp.run();
    QueuePair& queue = tcp.getQueue();
    for(;;)
    {
        MapMsg f = queue.timed_pop(2000);
        if(f.cmd())
        {
            try
            {
                if(f.cmd() == "quit")
                {
                    sleep(2); //FIXME: OUCH RACE COND ON QUIT!
                    THROW_ERROR("quit in tcpGetBuffer");
                }
                if(f.cmd() == "buffer")
                {
                    id = f["id"];
                    ret = static_cast<std::string>(f["str"]);
                    break;
                }
                else
                    LOG_INFO("Unknown msg.");
            }
            catch(ErrorExcept e)
            {
                if (f.cmd() == "quit")
                    throw e;
            }
        }
    }
    return ret;
}

#include <errno.h>

bool tcpSendBuffer(const std::string ip, int port, int id, const std::string caps)
{
    bool ret = false;
    LOG_INFO("got ip=" << ip << " port=" << port << " id=" << 
            id << " caps=" << caps);
    MapMsg msg("buffer");

    TcpServer tcp(port);
    msg["str"] = caps;
    msg["id"] = id;

    const int MAX_TRIES = 100;

    std::string msg_str;
    msg.stringify(msg_str);
    for(int i = 0; i < MAX_TRIES; ++i)
    {
        if(MsgThread::isQuitted())
            break;
        try
        {
            ret = tcp.socket_connect_send(ip, msg_str);
            if(ret)
                break;
        }
        catch(ErrorExcept e)
        {
           if(e.errno_ == ECONNREFUSED ) 
               LOG_DEBUG("GOT ECONNREFUSED");
           else
               break;
        }
        usleep(100000);
    }
    return ret;
}


