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
#define DEFAULT_PARAMETER "default"

class Session
{
    public:
        /*
         * Class construct. Create a new Session object.
         *
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
         * 
         * @param   r_uri	The remote SIP address. Default value: localhost on the port 5060
         *
         * @return  0 if the invite request has been successfully created and sent
         *          1 otherwise
         */
        virtual int connect( std::string r_uri = DEFAULT_PARAMETER ) = 0;

        /*
         * Terminate a current remote connection
         *
         * @return  0 if the request bye has been successfully created and sent
         *          1 otherwise
         */
        virtual int disconnect( ) = 0;

        /*
         * Free memory allocated by the pjsip library and by our C++ module
         *
         * @return  0 on success
         *          1 otherwise
         */
        virtual int shutdown() = 0;

        /*
         * Initialize the user agent module. This function has to be called before any function engaging pjsip
         *
         * @return  0 if everything went well
         *          1 otherwise
         */
        virtual int init() = 0;

        /*
         * Update the media parameters or the contact inside an existing dialog
         *
         * @return  0 if the request has been successfully created and sent
         *          1 otherwise
         */
        virtual int reinvite() = 0;

        /*
         * Send an instant message to the connected peer
         *
         * @param msg   The text message.
         *
         * @return  0 if the message was successfully sent
         *          1 if a problem occured 
         */
        virtual int send_instant_message( std::string msg ) = 0;
    
        virtual std::string get_message() = 0;

        virtual int get_connection_port() = 0;

        /*
         * Accept an incoming request. Equivalent to the pickup operation
         *
         * @return  0 on success
         *          1 if a problem occured
         */
        virtual int accept( void ) = 0;

        /*
         * Refuse an incoming request.
         *
         * @return  0 on success
         *          1 if a problem occured
         */
        virtual int refuse( void ) = 0;

        /*
         * Build a readable string form of the active media
         *
         * @return std::string  The media
         */
        virtual std::string media_to_string() = 0;

        /*
         * Add a media to the session. It will be used for SDP session
         *
         * @param type  The type of media (ex: audio, video, ... )
         * @param codecs    the formatted list of encoding codec names
         * @param port  The port to transport this media
         * @param dir   The stream direction ( ex: sendrecv, sendonly, ...)
         */
        virtual void set_media( std::string type, std::string codecs, int port,
                               std::string dir = DEFAULT_PARAMETER ) = 0;

        /*
         * Monitor the current connection state
         *
         * @return std::string  A string description of the current state
         */
        virtual std::string get_connection_state( void ) = 0;

        /*
         * Monitor the current error status
         *
         * @return std::string  A string description of the current error status
         */
        virtual std::string get_error_reason( void ) = 0;

        /*
         * Define the application behaviour on an incoming call
         *
         * @param mode  The answer mode: 0  Auto mode
         *                               1  Manual mode
         */
        virtual void set_answer_mode( int mode ) = 0;

        /*
         * Get the application behaviour on an incoming call
         *
         * @return std::string  The answer mode:   'auto'
         *                                         'manual'
         */
        virtual std::string get_answer_mode( void ) = 0;

        /*
         * Set the running python instance address to be able to call it back later
         *
         * @param p The python instance as a transparent PyObject
         */
        virtual void set_python_instance(PyObject *p) = 0;

        /*
         * Read accessor to the session listening port
         * Default: 5060
         */
        int get_session_port() { return _port; }

        /*
         * Write accessor to the session listening port
         * Default: 5060
         */
        void set_session_port(int port){ _port = port; }

    private:
        /* The communication protocol */
        int _protocol;

        /* The protocol listening port */
        int _port;
};

#endif // _SESSION_H
