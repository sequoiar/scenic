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

#ifndef _SIP_SESSION_H
#define _SIP_SESSION_H

/*
 * @file    sipsession.h
 * @brief	The SIP implementation
 */

#include <string>

#include "session.h"
#include "useragent.h"

#define _APP_NAME   "miville"
#define _DEFAULT_SIP_PORT   5060

class SIPSession
: public Session
{
    public:
        /*
         * Create a new SIP session object
         *
         * @param port	The local protocol listening port.
         */
        SIPSession( int port );

        /*
         * Class destructor
         */
        ~SIPSession();

        /*
         * Establishes a SIP connection with a remote host.
         *
         * @param   r_uri	The remote SIP address
         */
        int connect( std::string r_uri );

        /*
         * Terminate a current SIP remote connection
         */
        int disconnect();

        /*
         * Send an instant message
         */
        int sendInstantMessage( std::string msg );

        int accept( void );

        int refuse( int reason );

        void build_sdp( void );

        void startMainloop();

        /*
         * Add a media to the session. It will be used for SDP session
         *
         * @param codecs    the formatted list of encoding codec names
         *                  Pattern: separator: '/', must end with the separator
         */
        void addMedia( std::string codecs );

        SIPSession(SIPSession const&); //No Copy Constructor
        SIPSession& operator=(const SIPSession&); //No Assignment Operator

    private:

        /* The application SIP User Agent */
        UserAgent* _app_ua;

};

#endif // _SIP_SESSION_H
