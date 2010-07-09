/* MulticastCapsClient.h
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
 * Derived from receiver.cpp:
 * Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef _MULTICAST_CAPS_CLIENT_H_
#define _MULTICAST_CAPS_CLIENT_H_


#include <string>
#include <boost/asio.hpp>

class MulticastCapsClient
{
    public:
        MulticastCapsClient(boost::asio::io_service& io_service,
                const boost::asio::ip::address& listen_address,
                const boost::asio::ip::address& multicast_address);
        std::string getCaps();
    
    private:
        boost::asio::ip::udp::socket socket_;
        boost::asio::ip::address listen_address_;
        boost::asio::ip::address multicast_address_;
};

#endif // _MULTICAST_CAPS_CLIENT_H_
