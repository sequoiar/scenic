/* MulticastCapsServer.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Derived from sender.cpp:
 * Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include "multicast_caps_server.h"
#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include "util/sigint.h"

MulticastCapsServer::MulticastCapsServer(const std::string &multicast_address,
        short multicast_port,
        const std::string &message) :
    io_service_(),
    endpoint_(boost::asio::ip::address::from_string(multicast_address), multicast_port),
    socket_(io_service_, endpoint_.protocol()),
    timer_(io_service_),
    message_(message + std::string(" BYTES SENT: ") + boost::lexical_cast<std::string>(message.length())),
    serverThread_(boost::bind(&boost::asio::io_service::run, &io_service_)),
    done_(false)
{
    socket_.async_send_to(boost::asio::buffer(message_), endpoint_,
            boost::bind(&MulticastCapsServer::handle_send_to, this,
                boost::asio::placeholders::error));
}

MulticastCapsServer::~MulticastCapsServer()
{
    io_service_.stop();
    serverThread_.join();
}

void MulticastCapsServer::handle_send_to(const boost::system::error_code& error)
{
    if (!error and not done_)
    {
        static const int PERIOD = 2; // seconds
        timer_.expires_from_now(boost::posix_time::seconds(PERIOD));
        timer_.async_wait(
                boost::bind(&MulticastCapsServer::handle_timeout, this,
                    boost::asio::placeholders::error));
    }
}

void MulticastCapsServer::handle_timeout(const boost::system::error_code& error)
{
    if (!error and not done_)
    {
        done_ = signal_handlers::signalFlag();
        socket_.async_send_to(
                boost::asio::buffer(message_, message_.length()), endpoint_,
                boost::bind(&MulticastCapsServer::handle_send_to, this,
                    boost::asio::placeholders::error));
    }
}

