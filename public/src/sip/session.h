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

#ifndef _SESSION_H
#define _SESSION_H

/*
 * @file    session.h
 * @brief	An interface to expose to the core the available methods
 */

#include <string>

#include "useragent.h"

#define PROTOCOL_SIP    0

class Session
{
    public:
        /*
         * Class construct. Create a new Session object.
         * @param type	The protocol type. As for now, only SIP (Session Initiation Protocol) is available
         * @param port	The local protocol listening port.
         */
        Session( int type, int port )
            : _protocol( type ), _port( port )
        {
            // nothing else
        }


        /*
         * Class destructor
         */
        virtual ~Session() {}

        /*
         * Establishes a connection with a remote host.
         * @param   r_uri	The remote SIP address
         * @param   r_port	The remote connection port
         */
        virtual int connect( std::string r_uri ) = 0;

        /*
         * Terminate a current remote connection
         */
        virtual int disconnect( ) = 0;

        virtual int shutdown() = 0;

        virtual int reinvite() = 0;

        /*
         * Send an instant message
         */
        virtual int sendInstantMessage( std::string msg ) = 0;

        /*
         * Accept incoming request
         */
        virtual int accept( void ) = 0;

        virtual int refuse( int reason ) = 0;

        virtual void build_sdp() = 0;

        virtual std::string mediaToString() = 0;

        /*
         * Add a media to the session. It will be used for SDP session
         *
         * @param codecs    the formatted list of encoding codec names
         *                  Pattern: a=codec1/codec2/../codecn/ (a pour audio; v pour video)
         */
        virtual void addMedia( std::string type, std::string codecs, int port ) = 0;

        /* Read accessor. Return the protocol port */
        int getSessionPort( void ){ return _port; }

    private:
        /* The communication protocol */
        int _protocol;

        /* The protocol listening port */
        int _port;
};

#endif // _SESSION_H
