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

#define APP_NAME   "miville"

class SIPSession
    : public Session
{
    public:

        /*
         * Create a new SIP session object
         *
         * @param port	The local protocol listening port.
         */
        SIPSession( int port = DEFAULT_SIP_PORT );

        /*
         * Class destructor
         */
        ~SIPSession();

        /*
         * Establishes a SIP connection with a remote host.
         *
         * @param   r_uri	The remote SIP address
         *
         * @return  0 if the invite request has been successfully created and sent
         *          1 otherwise
         */
        int connect(  std::string r_uri );

        /*
         * Terminate a current remote connection
         *
         * @return  0 if the request bye has been successfully created and sent
         *          1 otherwise
         */
        int disconnect();

        /*
         * Free memory allocated by the pjsip library and by our C++ module
         *
         * @return  0 on success
         *          1 otherwise
         */
        int shutdown();

        /*
         * Initialize the user agent module. This function has to be called before any function engaging pjsip
         *
         * @return  0 if everything went well
         *          1 otherwise
         */
        int init();

        /*
         * Update the media parameters or the contact inside an existing dialog
         *
         * @return  0 if the request has been successfully created and sent
         *          1 otherwise
         */
        int reinvite();

        /*
         * Send an instant message to the connected peer
         *
         * @param msg   The text message.
         *
         * @return  0 if the message was successfully sent
         *          1 if a problem occured 
         */
        int send_instant_message( std::string msg );

        std::string get_message( void );

        /*
         * Accept an incoming request. Equivalent to the pickup operation
         *
         * @return  0 on success
         *          1 if a problem occured
         */
        int accept( void );

        /*
         * Refuse an incoming request.
         *
         * @return  0 on success
         *          1 if a problem occured
         */
        int refuse( void );

        /*
         * Build a readable string form of the active media
         *
         * @return std::string  The media
         */
        std::string media_to_string();

        /*
         * Add a media to the session. It will be used for SDP session
         *
         * @param type  The type of media (ex: audio, video, ... )
         * @param codecs    the formatted list of encoding codec names
         * @param port  The port to transport this media
         * @param dir   The stream direction ( ex: sendrecv, sendonly, ...)
         */
        void set_media( std::string type, std::string codecs, int port,
                       std::string dir = DEFAULT_PARAMETER );

        
        /*
         * Monitor the current connection state
         *
         * @return std::string  A string description of the current state
         */
        std::string get_connection_state( void );

        /*
         * Monitor the current error status
         *
         * @return std::string  A string description of the current error status
         */
        std::string get_error_reason( void );

        /*
         * Define the application behaviour on an incoming call
         *
         * @param mode  The answer mode: 0  Auto mode
         *                               1  Manual mode
         */
        void set_answer_mode( int mode );

        /*
         * Define the application behaviour on an incoming call
         *
         * @param mode  The answer mode: 0  Auto mode
         *                               1  Manual mode
         */
        std::string get_answer_mode( void );

        /*
         * Get the application behaviour on an incoming call
         *
         * @return std::string  The answer mode:   'auto'
         *                                         'manual'
         */
        void set_python_instance( PyObject *p );

        SIPSession(SIPSession const&); //No Copy Constructor
        SIPSession& operator=(const SIPSession&); //No Assignment Operator

    private:

        /* The application SIP User Agent */
        UserAgent* _app_ua;
        
};

#endif // _SIP_SESSION_H
