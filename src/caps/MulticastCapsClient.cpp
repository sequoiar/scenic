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

#include "util/logWriter.h"
#include "MulticastCapsClient.h"

const short multicast_port = 30001;

MulticastCapsClient::MulticastCapsClient(boost::asio::io_service& io_service,
        const boost::asio::ip::address& listen_address,
                const boost::asio::ip::address& multicast_address) : 
    socket_(io_service),
    listen_address_(listen_address),
    multicast_address_(multicast_address)
{}

std::string MulticastCapsClient::getCaps()
{
    boost::asio::ip::udp::endpoint sender_endpoint;
    // Create the socket so that multiple clients may be bound to the same address.
    boost::asio::ip::udp::endpoint listen_endpoint(
        listen_address_, multicast_port);
    socket_.open(listen_endpoint.protocol());
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.bind(listen_endpoint);

    // Join the multicast group.
    socket_.set_option(boost::asio::ip::multicast::join_group(multicast_address_));

    const int BUF_SIZE = 4096;
    char data[BUF_SIZE];

    bool done = false;
    while (not done)
    {
        size_t len = socket_.receive_from(boost::asio::buffer(data, BUF_SIZE), sender_endpoint);
        LOG_DEBUG("Received " << len << "bytes");
        // TODO: message is terminated by a null character, so once we have that we're done
        done = true;
    }
    //std::string retVal((std::istreambuf_iterator<char>(&buf)),
     //       std::istreambuf_iterator<char>());
    std::string retVal(data);
    return retVal;
}

