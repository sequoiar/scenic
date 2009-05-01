/* asioThread.cpp
 * Copyright (C) 2009 Société des arts technologiques (SAT)
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
 * uses code from 
 * async_tcp_echo_server.cpp
 * Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */



#include <cstdlib>
#include <iostream>
#include "util.h"
#include "asioThread.h"

#ifdef HAVE_BOOST_ASIO
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::placeholders::error;
using boost::asio::placeholders::bytes_transferred;

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

class tcp_session
{
    public:
        tcp_session(io_service& io_service, QueuePair& queue)
            : io_service_(io_service),socket_(io_service),queue_(queue), welcome_(),
            t_(io_service, boost::posix_time::millisec(1))
    {
        std::cout << "READY\n";
    }
        ~tcp_session()
        {
            io_service_.stop();
        }
        tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            socket_.async_read_some(buffer(data_, max_length),
                    boost::bind(&tcp_session::read_cb, this, 
                        error, bytes_transferred));
            t_.expires_at(t_.expires_at() + boost::posix_time::millisec(1));
            t_.async_wait(boost::bind(&tcp_session::handle_timer,this, error));
        }

        void read_cb(const error_code& err,
                size_t bytes_transferred)
        {
            if (!err)
            {
                std::string msgs(data_);
                std::string line = get_line(msgs);
                LOG_DEBUG(line);
                do
                {
                    MapMsg mapMsg;
                    if(mapMsg.tokenize(line))
                        queue_.push(mapMsg);
                    else
                        LOG_WARNING("Bad Msg Received.");
                    line = get_line(msgs);
                }
                while(!line.empty());
#if 0
                MapMsg mapMsg;
                if(mapMsg.tokenize(data_))
                    queue_.push(mapMsg);
                else
                    LOG_WARNING("Bad Msg Received.");
#endif
                memset(data_, 0, max_length);
                socket_.async_read_some(buffer(data_, max_length),
                        boost::bind(&tcp_session::read_cb, this, error, bytes_transferred));
            }
            else
            {
                std::cout << "here" << std::endl;
                delete this;
            }
        }

        void write_cb(const error_code& err)
        {
            if (!err)
            {
                t_.expires_at(t_.expires_at() + boost::posix_time::millisec(1));
                t_.async_wait(boost::bind(&tcp_session::handle_timer,this, error));
            }
            else
            {
                std::cout << "here2" << std::endl;
                delete this;
            }
        }

        void handle_timer(const error_code& err)
        {
            if (!err)
            {
                if(queue_.ready())
                {
                    MapMsg msg = queue_.timed_pop(1);
                    if(!msg.cmd().empty())
                    {
                        if(msg.cmd() == "quit")
                        {
                            LOG_DEBUG("quit goes here");
                        }
                        else
                        {
                            std::string msg_str;
                            msg.stringify(msg_str);
                            msg_str+="\x0D\x0A";
                            async_write(socket_, buffer(msg_str), boost::bind(&tcp_session::write_cb, this, error));
                        }
                    }
                    else
                        THROW_ERROR("queue.ready() but msg empty!");
                }
                else
                {
                    t_.expires_at(t_.expires_at() + boost::posix_time::millisec(MILLISEC_WAIT));
                    t_.async_wait(boost::bind(&tcp_session::handle_timer,this, error));
                }
            }
            else
            {
                std::cout << "here3" << std::endl;
            }
        }
    private:
        io_service& io_service_;
        tcp::socket socket_;
        enum { max_length = 1024 };
        char data_[max_length];
        QueuePair& queue_;
        std::string welcome_;
        boost::asio::deadline_timer t_;
};

class tcp_server
{
    public:
        tcp_server(io_service& io_service, short port, QueuePair& queue)
            : io_service_(io_service),
            acceptor_(io_service, tcp::endpoint(boost::asio::ip::address_v4::loopback(), port)), queue_(queue),
            t_(io_service, boost::posix_time::seconds(1))
    {
        tcp_session* new_tcp_session = new tcp_session(io_service_,queue_); 
        acceptor_.async_accept(new_tcp_session->socket(),
                boost::bind(&tcp_server::handle_accept, this, new_tcp_session, 
                    error));
        t_.async_wait(boost::bind(&tcp_server::handle_timer,this, error));
    }


        void handle_accept(tcp_session* new_tcp_session,
                const error_code& err)
        {
            if (!err)
            {
                new_tcp_session->start();
#if 0
                new_tcp_session = new tcp_session(io_service_,queue_);
                acceptor_.async_accept(new_tcp_session->socket(),
                        boost::bind(&tcp_server::handle_accept, this, new_tcp_session,
                            error));
#endif
            }
            else
            {
                std::cout << "here4" << std::endl;
                delete new_tcp_session;
            }
        }

        void handle_timer(const error_code& err)
        {
            if (!err)
            {
                t_.expires_at(t_.expires_at() + boost::posix_time::seconds(1));
                t_.async_wait(boost::bind(&tcp_server::handle_timer,this, error));
                if(MsgThread::isQuitted())
                    io_service_.stop();

            }
            else
            {
                std::cout << "here5" << std::endl;
            }
        }

    private:
        io_service& io_service_;
        tcp::acceptor acceptor_;
        QueuePair& queue_;
        boost::asio::deadline_timer t_;
};

class udp_sender
{
    public:
        udp_sender(io_service& io_service, std::string ip, std::string port, std::string buff)
            : io_service_(io_service),buff_(buff),t_(io_service, boost::posix_time::seconds(1)),
            socket_(io_service, udp::endpoint(udp::v4(), 0)),
            sender_endpoint_(),resolver(io_service),query(udp::v4(), ip.c_str(), port.c_str()),iterator(resolver.resolve(query))
    {
        t_.async_wait(boost::bind(&udp_sender::handle_timer,this, error));
    }

        void handle_send_to(const error_code& err, size_t bytes_sent)
        {
            if(err){
                LOG_DEBUG("err");
            } 
            else
            {
                LOG_DEBUG("MSG Sent " << bytes_sent);
                socket_.async_receive_from(buffer(data_,max_length),sender_endpoint_,
                        boost::bind(&udp_sender::handle_receive_from, this, 
                            error, bytes_transferred));
                t_.expires_at(t_.expires_at() + boost::posix_time::seconds(1));
                t_.async_wait(boost::bind(&udp_sender::handle_timer,this, error));
            }
        }

        void handle_receive_from(const error_code& err,
                size_t bytes_recvd)
        {
            if (!err && bytes_recvd > 0)
            {
                io_service_.stop();
                LOG_DEBUG("Got data:" << data_);
            }
            else
            {
                LOG_DEBUG("Got err or bytes = 0");
            }
        }

        void handle_timer(const error_code& err)
        {
            if (!err)
            {
                if(MsgThread::isQuitted())
                    io_service_.stop();
                
                socket_.async_send_to(buffer(buff_),*iterator, boost::bind(&udp_sender::handle_send_to,this,error,bytes_transferred));
            }
            else
            {
                std::cout << "here5" << std::endl;
            }
        }
        io_service& io_service_; 
        std::string buff_;
        boost::asio::deadline_timer t_;
        udp::socket socket_;
        udp::endpoint sender_endpoint_;
        udp::resolver resolver;
        udp::resolver::query query;
        udp::resolver::iterator iterator;
        enum { max_length = 1024 };
        char data_[max_length];
};

class udp_server
{
    public:
        udp_server(io_service& io_service, short port, std::string& buff,int& id)
            : io_service_(io_service),port_(port),buff_(buff),id_(id),
            socket_(io_service, udp::endpoint(udp::v4(), port)), sender_endpoint_(),t_(io_service, boost::posix_time::seconds(1))
    {
        socket_.async_receive_from(buffer(data_, max_length), sender_endpoint_, 
                boost::bind(&udp_server::handle_receive_from, this, 
                    error, bytes_transferred));
        t_.async_wait(boost::bind(&udp_server::handle_timer,this, error));
    }

        void handle_receive_from(const error_code& err,
                size_t bytes_recvd)
        {
            if (!err && bytes_recvd > 0)
            {
                MapMsg msg;
                LOG_DEBUG("MSG: bytes_recvd " << bytes_recvd);
                msg.tokenize(data_);
                if(msg.cmd() == "buffer")
                {
                    id_ = msg["id"];
                    buff_ = static_cast<std::string>(msg["str"]);
                    socket_.async_send_to(buffer("ok"), sender_endpoint_,
                            boost::bind(&udp_server::handle_send_to, this,
                                error, bytes_transferred));
                }
                else
                {
                    socket_.async_receive_from(buffer(data_, max_length), sender_endpoint_,
                            boost::bind(&udp_server::handle_receive_from, this,
                                error, bytes_transferred));

                }
            }
            else
            {
                socket_.async_receive_from(buffer(data_, max_length), sender_endpoint_,
                        boost::bind(&udp_server::handle_receive_from, this,
                            error, bytes_transferred));
            }
        }

        void handle_send_to(const error_code& , size_t )//bytes_sent)
        {
            LOG_DEBUG("DONE");
            io_service_.stop();
        }

        void handle_timer(const error_code& err)
        {
            if (!err)
            {
                if(MsgThread::isQuitted())
                {
                    io_service_.stop();
                }
                t_.expires_at(t_.expires_at() + boost::posix_time::seconds(1));
                t_.async_wait(boost::bind(&udp_server::handle_timer,this, error));
            }
            else
            {
                std::cout << "ERRRR" << std::endl;
            }
        }
    private:
        io_service& io_service_; 
        short port_; std::string& buff_;int& id_;
        udp::socket socket_;
        udp::endpoint sender_endpoint_;
        enum { max_length = 1024*8 };
        char data_[max_length];
        boost::asio::deadline_timer t_;
};


std::string tcpGetBuffer(int port, int &id)
{
    std::string buff;
    io_service io_service;
    id = 0;
    udp_server us(io_service, port,buff,id);
    io_service.run();
    if(id == 0)
        THROW_ERROR("Bad id");

    return buff;
}
#include <sstream>
bool tcpSendBuffer(std::string ip, int port, int id, std::string caps)
{
    bool ret = false;
    LOG_INFO("got ip=" << ip << " port=" << port << " id=" << 
            id << " caps=" << caps);
    MapMsg msg("buffer");

    msg["str"] = caps;
    msg["id"] = id;
    std::ostringstream str;
    str << port;
    std::string msg_str;
    msg.stringify(msg_str);
    LOG_DEBUG("!!!!!!!!"<<str);
    io_service io_service;


    udp_sender us(io_service,ip,str.str(),  msg_str);
    io_service.run();
    ret = true;
    return ret;
}

#endif



void asio_thread::main()
{


    try
    {
#ifdef HAVE_BOOST_ASIO
        io_service io_service;

        tcp_server s(io_service, port_,queue_);

        io_service.run();
#endif
    }
    catch (Except)
    {
    }

}

