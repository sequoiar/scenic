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
using boost::asio::buffer;
using namespace boost::posix_time;

using boost::asio::ip::tcp;
static const std::string SENTINEL = "END_BUFFER";
static const int POLL_QUIT_MS = 1;

// shared_from_this gives shared_ptr to this, this way we cleanly avoid memory leaks
class ReceiverSession : public boost::enable_shared_from_this<ReceiverSession> {
    public:
        ReceiverSession(boost::asio::io_service& io_service, std::string *receiverBuffer) : 
            socket_(io_service), 
            receiverBuffer_(receiverBuffer),
            timer_(io_service, millisec(POLL_QUIT_MS))
    {
        // periodically check if we've been quit/interrupted, every millisecond
        timer_.async_wait(boost::bind(&ReceiverSession::handle_timer, this, boost::asio::placeholders::error));
    }

        tcp::socket& socket()
        {
            return socket_;
        }

        void start()
        {
            socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                    boost::bind(&ReceiverSession::handle_receive_from, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void handle_timer(const boost::system::error_code& err)
        {
            if (!err)
            { 
                // schedule this handler to be called again in 1 second
                timer_.expires_at(timer_.expires_at() + seconds(1));
                timer_.async_wait(boost::bind(&ReceiverSession::handle_timer, this, boost::asio::placeholders::error));

                if (signal_handlers::signalFlag())
                {
                    socket_.get_io_service().stop();
                    // FIXME: caps server will have to be more elegant than this
                    THROW_ERROR("INTERRUPTED");
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
                os.write(data_.data(), bytes_transferred);

                *receiverBuffer_ += os.str();

                // LOG_DEBUG("receiver buffer looks like " << receiverBuffer_);
                // Got the quit message, now stop the service
                std::string::size_type sentinelPos = receiverBuffer_->find(SENTINEL);
                if (sentinelPos != std::string::npos)   // end of message
                {
                    LOG_DEBUG("Got " << SENTINEL);
                    receiverBuffer_->resize(sentinelPos);    // strip quit from caps
                    socket_.get_io_service().stop();
                }
                else
                {
                    socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                            boost::bind(&ReceiverSession::handle_receive_from, shared_from_this(),
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
        enum { MAX_LENGTH = 8000 };
        boost::array<char, MAX_LENGTH> data_;
        std::string *receiverBuffer_;
        boost::asio::deadline_timer timer_;
};


class TcpReceiver {
    typedef boost::shared_ptr<ReceiverSession> session_ptr;

    public:
    TcpReceiver(boost::asio::io_service& io_service, int port, std::string *buffer) : 
        io_service_(io_service),
        acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
        buffer_(buffer)
    {
        session_ptr new_session(new ReceiverSession(io_service_, buffer_));
        acceptor_.async_accept(new_session->socket(),
                boost::bind(&TcpReceiver::handle_accept, this, new_session,
                    boost::asio::placeholders::error));
    }

    void handle_accept(session_ptr new_session,
            const boost::system::error_code& error)
    {
        if (!error)
        {
            new_session->start();
            new_session.reset(new ReceiverSession(io_service_, buffer_));
            acceptor_.async_accept(new_session->socket(),
                    boost::bind(&TcpReceiver::handle_accept, this, new_session,
                        boost::asio::placeholders::error));
        }
        else
            LOG_WARNING("Got error" << boost::system::system_error(error).what());
    }

    private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    std::string *buffer_;
};


std::string asio::tcpGetBuffer(int port)
{
    std::string buffer;
    boost::asio::io_service io_service;

    TcpReceiver receiver(io_service, port, &buffer);
    LOG_DEBUG("Waiting for msg on port " << port);

    io_service.run();

    LOG_DEBUG("Got buffer " << buffer);
    return buffer;
}


bool asio::tcpSendBuffer(const std::string &ip, int port, const std::string &caps)
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

        std::string msg(caps);
        msg += SENTINEL;
        LOG_DEBUG("Sending " << msg.length() << " characters");
        LOG_DEBUG(msg);
        boost::asio::write(s, boost::asio::buffer(msg, msg.length()));
        success = true;
    }
    catch (const std::exception& e)
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

