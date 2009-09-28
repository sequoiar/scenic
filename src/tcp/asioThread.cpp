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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#pragma GCC diagnostic ignored "-Weffc++"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;
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

#if 0

class udp_sender
{
    public:
        udp_sender(io_service& io_service, std::string ip, std::string port, std::string buff)
            : io_service_(io_service), 
            buff_(buff), 
            t_(io_service, seconds(1)),
            socket_(io_service, udp::endpoint(udp::v4(), 0)), 
            sender_endpoint_(),
            resolver(io_service),
            query(udp::v4(), ip.c_str(), port.c_str()), iterator(resolver.resolve(query))
    {
        t_.async_wait(boost::bind(&udp_sender::handle_timer, this, error));
    }

        void handle_send_to(const error_code& err, size_t bytes_sent)
        {
            if (err)
            {
                LOG_DEBUG("err");
            } 
            else
            {
                LOG_DEBUG("MSG Sent " << bytes_sent);
                socket_.async_receive_from(buffer(data_,max_length),sender_endpoint_,
                        boost::bind(&udp_sender::handle_receive_from, this, 
                            error, bytes_transferred));

                t_.expires_at(t_.expires_at() + seconds(1));
                t_.async_wait(boost::bind(&udp_sender::handle_timer, this, error));
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

                socket_.async_send_to(buffer(buff_), *iterator, 
                        boost::bind(&udp_sender::handle_send_to, this, error, bytes_transferred));
            }
            else
            {
                std::cout << "here5" << std::endl;
            }
        }

    private:
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
            : io_service_(io_service),
            port_(port),
            buff_(buff),
            id_(id),
            socket_(io_service, udp::endpoint(udp::v4(), port)), 
            sender_endpoint_(),
            t_(io_service, seconds(1))
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
                if(msg() == "buffer")
                {
                    id_ = msg["id"];
                    buff_ = msg["str"].str();
                    socket_.async_send_to(buffer("ok"), sender_endpoint_,
                            boost::bind(&udp_server::handle_send_to, this,
                                error, bytes_transferred));

                    return;
                }
            }
            socket_.async_receive_from(buffer(data_, max_length), sender_endpoint_,
                    boost::bind(&udp_server::handle_receive_from, this,
                        error, bytes_transferred));
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
                t_.expires_at(t_.expires_at() + seconds(1));
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

#endif

using boost::asio::ip::tcp;

class tcp_receiver_session : 
    public boost::enable_shared_from_this<tcp_receiver_session>
{
    public:
        tcp_receiver_session(boost::asio::io_service& io_service, std::string &receiverBuffer)
            : socket_(io_service), receiverBuffer_(receiverBuffer)
        {}

        tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            // shared_from_this gives shared_ptr to this, this way we cleanly avoid memory leaks
            socket_.async_read_some(boost::asio::buffer(data_),
                    boost::bind(&tcp_receiver_session::handle_receive_from, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void handle_receive_from(const boost::system::error_code& error,
                size_t bytes_transferred)
        {
            if (!error)
            {
                LOG_DEBUG("received " << bytes_transferred << " bytes");

                std::ostringstream os;
                os.write(data_, bytes_transferred);

                // Got the message, now stop the service
                socket_.get_io_service().stop();
                receiverBuffer_ = os.str();
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
    std::string buffer;
    boost::asio::io_service io_service;

    LOG_DEBUG("Waiting for msg on port " << port);
    tcp_receiver receiver(io_service, port, buffer);

    io_service.run();

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

        enum {SIZE = 6000};
        boost::asio::write(s, boost::asio::buffer(caps));
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
        tcp_server s(io_service, port_,queue_);

        io_service.run();
    }
    catch (std::exception e)
    {
        LOG_ERROR("caught exception:" << e.what());
        queue_.push(MapMsg("quit"));
    }

}

#endif
