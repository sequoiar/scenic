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

#ifndef _USER_AGENT_H
#define _USER_AGENT_H

#define THIS_FILE       "useragent.cpp"
#define _LOCAL_IP_ADDRESS   "127.0.0.1"

/* @file	useragent.h
 * @brief	A SIP useragent. Implements the SIP stacks from the transaction layer to the transport layer as described in
 *      RFC 3261.
 */

#include <pjsip.h>
#include <pjlib.h>
#include <pjsip_ua.h>
#include <pjlib-util.h>

#include <string>

#include "sdp.h"

class UserAgent
{
    public:
        /*
         * Create a new UserAgent object
         * @param	name	The application name
         */
        UserAgent( std::string name, int port );

        /*
         * Class destructor
         */
        ~UserAgent();

        /*
         * Initialize all the mandatory data structures from the PJSIP library
         *
         * @param port	The port on which the user agent will listen
         *
         * @return int	PJ_SUCCESS on success
         */
        int init_pjsip_modules( );

        /*
         * Create an invite session. Handle the related incoming responses
         *
         * @param	uri	The SIP address to create connection with
         * @param	port		The remote SIP port
         *
         * @return  int	PJ_SUCCESS on success
         */
        int create_invite_session( std::string uri, int port );

        pj_str_t build_contact_uri( std::string user, int port );

        /*
         * Start the main loop event
         */
        void listen( void );

        void buildSDP( void );

        void addMediaToSession( int mime_type, std::string codecs );
    private:
        /* The module name */
        std::string _name;

        /* The local IP address */
        std::string _localIP;

        /* The local SIP port */
        int _lport;

        /*
         * Initialize the pjsip_module structure
         */
        void init_sip_module( void );
};

#endif // _USER_AGENT_H
