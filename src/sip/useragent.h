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

#define MSG_RINGING                 180
#define MSG_OK                      200
#define MSG_METHOD_NOT_ALLOWED      405
#define MSG_NOT_ACCEPTABLE_HERE     488
#define MSG_SERVER_INTERNAL_ERROR   500


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
#include "uri.h"

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
         *              Pattern: <sip:host@ip:port>
         *
         * @return  int	PJ_SUCCESS on success
         */
        int create_invite_session( std::string uri );

        /*
         * Start the main loop event
         */
        void listen( void );

        void addMediaToSession( std::string codecs );

        URI* getLocalURI() { return _localURI; }

    private:
        /* The module name */
        std::string _name;

        /* The local SIP address */
        URI * _localURI;

        /*
         * Initialize the pjsip_module structure
         */
        void init_sip_module( void );

        UserAgent(const UserAgent&); //No Copy Constructor
        UserAgent& operator=(const UserAgent&); //No Assignment Operator
};

#endif // _USER_AGENT_H
