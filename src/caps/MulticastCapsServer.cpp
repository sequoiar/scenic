/* MulticastCapsServer.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
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
 * Derived from sender.cpp:
 * Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include "MulticastCapsServer.h"
#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

const short multicast_port = 30001;

MulticastCapsServer::MulticastCapsServer(const boost::asio::ip::address& multicast_address) : 
    io_service_(),
    endpoint_(multicast_address, multicast_port),
    socket_(io_service_, endpoint_.protocol()),
    timer_(io_service_), message_("These are multicast caps"),
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
    if (!error && !done_)
    {
        timer_.expires_from_now(boost::posix_time::seconds(1));
        timer_.async_wait(
                boost::bind(&MulticastCapsServer::handle_timeout, this,
                    boost::asio::placeholders::error));
    }
}

void MulticastCapsServer::handle_timeout(const boost::system::error_code& error)
{
    if (!error)
    {
        socket_.async_send_to(
                boost::asio::buffer(message_), endpoint_,
                boost::bind(&MulticastCapsServer::handle_send_to, this,
                    boost::asio::placeholders::error));
    }
}

#if 0
// example usage
int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: sender <multicast_address>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    sender 239.255.0.1\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    sender ff31::8000:1234\n";
            return 1;
        }
        sender s(boost::asio::ip::address::from_string(argv[1]));
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
#endif

