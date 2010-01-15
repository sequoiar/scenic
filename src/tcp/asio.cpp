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
#include "asio.h"

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
using boost::asio::buffer;
using boost::asio::placeholders::error;
using boost::asio::placeholders::bytes_transferred;
using namespace boost::posix_time;

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
                if (signal_handlers::signalFlag())
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


std::string asio::tcpGetBuffer(int port, int &/*id*/)
{
    std::string buffer("");
    boost::asio::io_service io_service;

    tcp_receiver receiver(io_service, port, buffer);
    LOG_DEBUG("Waiting for msg on port " << port);

    io_service.run();

    LOG_DEBUG("Got buffer " << buffer);
    return buffer;
}


bool asio::tcpSendBuffer(std::string ip, int port, int /*id*/, std::string caps)
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

