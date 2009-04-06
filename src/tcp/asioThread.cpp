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
#include "parser.h"
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


class tcp_session
{
    public:
        tcp_session(io_service& io_service, QueuePair& queue)
            : socket_(io_service),queue_(queue), welcome_(),
            t_(io_service, boost::posix_time::millisec(1)),msg_str()
        {
        }

        tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            welcome_ = "welcome.\nREADY:\n"; 
            async_write(socket_, buffer(welcome_),
                    boost::bind(&tcp_session::write_cb, this,
                        error));
        }

        void read_cb(const error_code& err,
                size_t bytes_transferred)
        {
            if (!err)
            {
                MapMsg mapMsg;
                if(Parser::tokenize(data_, mapMsg))
                    queue_.push(mapMsg);
                else
                    LOG_WARNING("Bad Msg Received.");

                socket_.async_read_some(buffer(data_, max_length),
                        boost::bind(&tcp_session::read_cb, this, 
                            error, bytes_transferred));
            }
            else
            {
                t_.cancel();
                delete this;
            }
        }

        void write_cb(const error_code& err)
        {
            if (!err)
            {
                socket_.async_read_some(buffer(data_, max_length),
                        boost::bind(&tcp_session::read_cb, this, 
                            error, bytes_transferred));
                t_.expires_at(t_.expires_at() + boost::posix_time::millisec(1));
                t_.async_wait(boost::bind(&tcp_session::handle_timer,this, error));
            }
            else
            {
                t_.cancel();
                delete this;
            }
        }

        void handle_timer(const error_code& err)
        {
            if (!err)
            {
                MapMsg msg;
                msg = queue_.timed_pop(1);
                if(!msg.cmd().empty())
                {
                    Parser::stringify(msg, msg_str);
                    msg_str+='\n';
                    async_write(socket_, buffer(msg_str),
                            boost::bind(&tcp_session::write_cb, this,
                                error));
                }
                else
                {
                    t_.expires_at(t_.expires_at() + boost::posix_time::millisec(1));
                    t_.async_wait(boost::bind(&tcp_session::handle_timer,this, error));
                }
            }
        }
    private:
        tcp::socket socket_;
        enum { max_length = 1024 };
        char data_[max_length];
        QueuePair& queue_;
        std::string welcome_;
        boost::asio::deadline_timer t_;
        std::string msg_str;
};

class tcp_server
{
    public:
        tcp_server(io_service& io_service, short port, QueuePair& queue)
            : io_service_(io_service),
            acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), queue_(queue),
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
                new_tcp_session = new tcp_session(io_service_,queue_);
                acceptor_.async_accept(new_tcp_session->socket(),
                        boost::bind(&tcp_server::handle_accept, this, new_tcp_session,
                            error));
            }
            else
            {
                delete new_tcp_session;
            }
        }

        void handle_timer(const error_code& /*error*/)
        {
            t_.expires_at(t_.expires_at() + boost::posix_time::seconds(1));
            t_.async_wait(boost::bind(&tcp_server::handle_timer,this, error));
            if(MsgThread::isQuitted())
                THROW_END_THREAD("bye");
        }

    private:
        io_service& io_service_;
        tcp::acceptor acceptor_;
        QueuePair& queue_;
        boost::asio::deadline_timer t_;
};


class udp_server
{
    public:
        udp_server(io_service& io_service, short port)
            : io_service_(io_service),
            socket_(io_service, udp::endpoint(udp::v4(), port)), sender_endpoint_()
    {
        socket_.async_receive_from(buffer(data_, max_length), sender_endpoint_, 
                boost::bind(&udp_server::handle_receive_from, this, 
                    error, bytes_transferred));
    }

        void handle_receive_from(const error_code& err,
                size_t bytes_recvd)
        {
            if (!err && bytes_recvd > 0)
            {
                socket_.async_send_to(buffer(data_, bytes_recvd), sender_endpoint_,
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

        void handle_send_to(const error_code& , size_t )//bytes_sent)
        {
            socket_.async_receive_from(buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&udp_server::handle_receive_from, this,
                        error, bytes_transferred));
        }

    private:
        io_service& io_service_;
        udp::socket socket_;
        udp::endpoint sender_endpoint_;
        enum { max_length = 1024 };
        char data_[max_length];
};
#endif



int asio_thread::main()
{


    try
    {
#ifdef HAVE_BOOST_ASIO
        io_service io_service;

        using namespace std; // For atoi.
        tcp_server s(io_service, port_,queue_);
        udp_server us(io_service, port_);

        io_service.run();
#endif
    }
    catch (Except)
    {
    }

    return 0;
}

