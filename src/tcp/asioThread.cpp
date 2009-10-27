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

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#pragma GCC diagnostic ignored "-Weffc++"

using boost::asio::ip::tcp;
using boost::asio::io_service;
using boost::system::error_code;
using boost::asio::async_write;
using boost::asio::buffer;
using boost::asio::placeholders::error;
using boost::asio::placeholders::bytes_transferred;
using boost::asio::ip::address_v4;
using namespace boost::posix_time;


///pull first line from msg  -returns first line 
static std::string get_line(std::string& msg)
{
    std::string ret=msg;
    std::string::size_type pos = msg.find_first_of("\r\n");
    if(pos != std::string::npos)
    {
        int count = 1;
        if(msg[pos+1] == '\r' or msg[pos+1] == '\n')
            count++;
        ret = msg.substr(0, pos+count);
        msg = msg.substr(pos+count);
        if(ret.find_first_not_of(" \r\n\t") == std::string::npos)
            ret.clear();
    }
    else
    {
        msg.clear();
        ret.clear();
    }

    return ret;
}

///The session once connected -- session must send READY
class tcp_session
{
    public:
        tcp_session(io_service& io_service, QueuePair& queue)
            : io_service_(io_service),
            socket_(io_service),
            queue_(queue), 
            welcome_(),
            t_(io_service, millisec(1))
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
            t_.expires_at(t_.expires_at() + millisec(1));
            t_.async_wait(boost::bind(&tcp_session::handle_timer,this, error));
        }

        void read_cb(const error_code& err,
                size_t bytes_transferred)
        {
            if (!err)
            {
                std::string msgs(data_);
                std::string line = get_line(msgs);
                do
                {
                    try
                    {
                        MapMsg mapMsg;
                        mapMsg.tokenize(line);
                        queue_.push(mapMsg);
                    }
                    catch(ErrorExcept)
                    {
                       LOG_DEBUG("Could not tokenize string --- continuing"); 
                    }
                    line = get_line(msgs);
                }
                while(!line.empty());

                memset(data_, 0, max_length);
                socket_.async_read_some(buffer(data_, max_length),
                        boost::bind(&tcp_session::read_cb, this, error, bytes_transferred));
            }
            else
            {
                queue_.push(MapMsg("quit"));
                std::cout << "here" << std::endl;
                delete this;
            }
        }

        void write_cb(const error_code& err)
        {
            if (!err)
            {
                t_.expires_at(t_.expires_at() + millisec(1));
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
                    if(msg())
                    {
                        if(!(msg() == "quit"))
                        {
                            std::string msg_str;
                            msg.stringify(msg_str);
                            msg_str+="\x0D\x0A";
                            async_write(socket_, buffer(msg_str), 
                                    boost::bind(&tcp_session::write_cb, this, error));
                        }
                    }
                    else
                        THROW_ERROR("queue.ready() but msg empty!");
                }
                else
                {
                    t_.expires_at(t_.expires_at() + millisec(MILLISEC_WAIT));
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
            acceptor_ ( io_service, 
                    tcp::endpoint(address_v4::loopback(), //Note: loopback only endpoint 
                        port)), 
            queue_(queue), 
            t_(io_service, seconds(1))
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
                new_tcp_session->start();
            else
                delete new_tcp_session;
        }

        void handle_timer(const error_code& err)
        {
            if (!err)
            {
                t_.expires_at(t_.expires_at() + seconds(1));
                t_.async_wait(boost::bind(&tcp_server::handle_timer,this, error));
                if(MsgThread::isQuitted())
                    io_service_.stop();
            }
        }

    private:
        io_service& io_service_;
        tcp::acceptor acceptor_;
        QueuePair& queue_;
        boost::asio::deadline_timer t_;
};

using boost::asio::ip::tcp;
static const std::string SENTINEL = "END_BUFFER";

class tcp_receiver_session : 
    public boost::enable_shared_from_this<tcp_receiver_session>
{
    public:
        tcp_receiver_session(boost::asio::io_service& io_service, std::string &receiverBuffer)
            : socket_(io_service), receiverBuffer_(receiverBuffer),
            timer_(io_service, millisec(1))
        {
            // periodically check if we've been quit/interrupted
            timer_.async_wait(boost::bind(&tcp_receiver_session::handle_timer, this, error));
        }

        tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            // shared_from_this gives shared_ptr to this, this way we cleanly avoid memory leaks
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                    boost::bind(&tcp_receiver_session::handle_receive_from, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }
        
        void handle_timer(const error_code& err)
        {
            if (!err)
            {
                timer_.expires_at(timer_.expires_at() + seconds(1));
                timer_.async_wait(boost::bind(&tcp_receiver_session::handle_timer, this, error)); // schedule this check for later
                if (MsgThread::isQuitted())
                {
                    socket_.get_io_service().stop();
                    LOG_ERROR("Interrupted while waiting to receive caps");
                }
            }
        }

        void handle_receive_from(const boost::system::error_code& error,
                size_t bytes_transferred)
        {
            if (!error)
            {
                LOG_DEBUG("received " << bytes_transferred << " bytes");

                std::ostringstream os;
                os.write(data_, bytes_transferred);

                receiverBuffer_ += os.str();

                LOG_DEBUG("receiver buffer looks like " << receiverBuffer_);
                // Got the quit message, now stop the service
                std::string::size_type sentinelPos = receiverBuffer_.find(SENTINEL);
                if (sentinelPos != std::string::npos)   // end of message
                {
                    LOG_DEBUG("Got " << SENTINEL);
                    receiverBuffer_.resize(sentinelPos);    // strip quit from caps
                    socket_.get_io_service().stop();
                }
                else
                {
                    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            boost::bind(&tcp_receiver_session::handle_receive_from, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
                }
            }
            else
                LOG_WARNING("Got error" << boost::system::system_error(error).what());
        }

    private:
        tcp::socket socket_;
        // FIXME: is this the best way of having a buffer? see boost/asio/examples/reference_counted.cpp
        enum { max_length = 8000 };
        char data_[max_length];
        std::string &receiverBuffer_;
        boost::asio::deadline_timer timer_;
};



class tcp_receiver
{
    typedef boost::shared_ptr<tcp_receiver_session> session_ptr;

    public:
    tcp_receiver(boost::asio::io_service& io_service, int port, std::string &buffer) : 
        io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        buffer_(buffer)
    {
        session_ptr new_session(new tcp_receiver_session(io_service_, buffer_));
        acceptor_.async_accept(new_session->socket(),
                boost::bind(&tcp_receiver::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
    }

    void handle_accept(session_ptr new_session,
            const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session.reset(new tcp_receiver_session(io_service_, buffer_));
            acceptor_.async_accept(new_session->socket(),
                    boost::bind(&tcp_receiver::handle_accept, this, new_session,
                        boost::asio::placeholders::error));
        }
        else
            LOG_WARNING("Got error" << boost::system::system_error(error).what());
    }

    private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    std::string &buffer_;
};


std::string tcpGetBuffer(int port, int &/*id*/)
{
    std::string buffer("");
    boost::asio::io_service io_service;

    tcp_receiver receiver(io_service, port, buffer);
    LOG_DEBUG("Waiting for msg on port " << port);

    io_service.run();

    LOG_DEBUG("Got buffer " << buffer);
    return buffer;
}


bool tcpSendBuffer(std::string ip, int port, int /*id*/, std::string caps)
{
    using boost::lexical_cast;

    boost::system::error_code err;
    bool success = false;

    try
    {
        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), ip, lexical_cast<std::string>(port));
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        err = s.connect(*iterator, err);

        LOG_DEBUG("Sending " << caps.length() << " characters");
        boost::asio::write(s, boost::asio::buffer(caps + SENTINEL, caps.length() + SENTINEL.length()));
        success = true;
    }
    catch (std::exception& e)
    {
        if (err != boost::asio::error::connection_refused)  // don't throw exception if connection isn't there
        {
            throw e;
        }
        else
        {
            LOG_DEBUG("No tcp receiver ready yet, will try again later...");
        }
    }
    return success;
}



void asio_thread::main()
{

    try
    {
        io_service io_service;
        tcp_server s(io_service, port_, queue_);

        io_service.run();
    }
    catch (std::exception e)
    {
        LOG_ERROR("caught exception:" << e.what());
        queue_.push(MapMsg("quit"));
    }

}

