/*
 * Copyright (C) 2008 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is free software: you can redistribute it and*or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Sropulpof is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Sropulpof.  If not, see <http:*www.gnu.org*licenses*>.
 */

#ifndef _SIP_URI
#define _SIP_URI

#include <string>

/*
 * @file uri.h
 * @brief   A class to manage SIP formatted address
 */

class URI
{
    public:
        /*
         * Class constructor.
         * Build an uri with the specified port and with the local address
         */
        URI( int port );
        
        /*
         * Class constructor.
         * Retrieve every fields from the string description and build the object 
         */
        URI( std::string uri );

        /*
         * Build an uri 
         */
        URI( std::string hostname, std::string hostip, int port );

        /*
         * Class destructor
         */
        ~URI();

        /*
         * Read accessor. Return the host name
         */
        std::string get_host_name(){ return _host_name; }
        
        /*
         * Read accessor. Return the host ip address
         */
        std::string get_hostip(){ return _host_ip; }
        
        /*
         * Read accessor. Return the port
         */
        int get_port(){ return _port; }

        /*
         * Read accessor. Return the string description of the built uri 
         */
        std::string get_address();

        /*
         * Read accessor. Return the string description of the default uri 
         */
        std::string build_default_local_uri( void );
        
        void to_string();

    private:
        // The host name
        std::string _host_name;
        // the host address IP
        std::string _host_ip;
        // The port
        int _port;

        friend class UserAgent;
};

#endif // _SIP_URI
