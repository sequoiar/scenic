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
#include <boost/lexical_cast.hpp>

MulticastCapsClient::MulticastCapsClient(boost::asio::io_service& io_service,
        const std::string& listenAddress,
        const std::string& multicastAddress, 
        short multicastPort) : 
    socket_(io_service),
    listenAddress_(boost::asio::ip::address::from_string(listenAddress)),
    multicastAddress_(boost::asio::ip::address::from_string(multicastAddress)),
    multicastPort_(multicastPort)
{}

std::string MulticastCapsClient::getCaps()
{
    boost::asio::ip::udp::endpoint sender_endpoint;
    // Create the socket so that multiple clients may be bound to the same address.
    boost::asio::ip::udp::endpoint listen_endpoint(
        listenAddress_, multicastPort_);
    socket_.open(listen_endpoint.protocol());
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.bind(listen_endpoint);

    // Join the multicast group.
    try {
    socket_.set_option(boost::asio::ip::multicast::join_group(multicastAddress_));
    }
    catch (const boost::system::system_error& error)
    {
        LOG_ERROR("Got socket error \"" << error.what() << 
                "\" for multicast address " << multicastAddress_);
        throw;
    }

    const int BUF_SIZE = 8192;
    char data[BUF_SIZE];

    bool done = false;
    std::string result;
    std::string::size_type capsEnd;
    const static std::string SENTINEL = "BYTES SENT: ";
    unsigned int bytesDeclared = 0;

    while (not done)
    {
        size_t len = socket_.receive_from(boost::asio::buffer(data, BUF_SIZE), sender_endpoint);
        LOG_DEBUG("Received " << len << "bytes");
        // message is terminated by a BYTES SENT: n, where n is the length of the caps,
        // so once we have that we're done
        result = std::string(data, len);
        capsEnd = result.find(SENTINEL);
        bytesDeclared = boost::lexical_cast<int>(result.substr(capsEnd + SENTINEL.length(), result.length()));
        if (bytesDeclared != capsEnd - 1)
            LOG_WARNING("MISMATCH BETWEEN DECLARED CAPS LENGTH " << 
                    bytesDeclared <<  " AND RECEIVED CAPS LENGTH" <<
                    capsEnd - 1);
        if (capsEnd != std::string::npos)
            done = true;
    }
    //std::string retVal((std::istreambuf_iterator<char>(&buf)),
     //       std::istreambuf_iterator<char>());
    return result.substr(0, capsEnd);
}

