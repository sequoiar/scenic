/* MulticastCapsServer.h
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

#ifndef _MULTICAST_CAPS_SERVER_H_
#define _MULTICAST_CAPS_SERVER_H_


#include <string>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

class MulticastCapsServer
{
    public:
        MulticastCapsServer(const boost::asio::ip::address& multicast_address);
        ~MulticastCapsServer();
        void handle_send_to(const boost::system::error_code& error);
        void handle_timeout(const boost::system::error_code& error);
    
    private:
        boost::asio::io_service io_service_;
        boost::asio::ip::udp::endpoint endpoint_;
        boost::asio::ip::udp::socket socket_;
        boost::asio::deadline_timer timer_;
        std::string message_;
        boost::thread serverThread_;
        bool done_;
};

#endif
