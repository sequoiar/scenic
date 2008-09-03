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

#define THIS_FILE       "useragent"
#define _LOCAL_IP_ADDRESS   "127.0.0.1"
#define PJ_LOG_LEVEL        5

#define MSG_RINGING                 180
#define MSG_OK                      200
#define MSG_METHOD_NOT_ALLOWED      405
#define MSG_NOT_ACCEPTABLE_HERE     488
#define MSG_SERVER_INTERNAL_ERROR   500

#define READY_TO_CONNECT    4
#define REQUEST_TIMEOUT     "Request Timeout"

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
#include "instantmessaging.h"

enum connectionState {
    CONNECTION_STATE_NULL,
    CONNECTION_STATE_CONNECTING,
    CONNECTION_STATE_INCOMING,
    CONNECTION_STATE_CONNECTED,
    CONNECTION_STATE_DISCONNECTED,
    CONNECTION_STATE_TIMEOUT
};

typedef enum connectionState connectionState;


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
        int inv_session_create( std::string uri );

	/*
         * Terminate an invite session. Send a bye to connected peer.
         *
         * @return int	0 on success
	 *              1 otherwise
         */
        int inv_session_end();

        /*
         * Send a invite inside an existing invite session.
         * Used for updating or modifying the media offer for instance. 
         * A new SDP negociation is done before validating the reinvite
         *
         * @return int	0 on success
	 *              1 otherwise
         */
        int inv_session_reinvite();

        /*
         * Accept an incoming call. Called when INVITE_AUTO_ANSWER = false
         *
         * @return int	0 on success
	 *              1 otherwise
         */
        int inv_session_accept();
        
        /*
         * Refuse an incoming call. Called when INVITE_AUTO_ANSWER = false
         *
         * @return int	0 on success
	 *              1 otherwise
         */
        int inv_session_refuse();

        /*
         * Set the media offer for the session
         *
         * @param type	The media type. For instance: "audio"
         * @param codecs  The list of the codec to use for the session. The codecs must
	 *                be separated with / and the last character must be / 
         * @param port The port to transport the media
         */
        void setSessionMedia( std::string type, std::string codecs, int port, std::string dir );

        /*
         * Return the local sip address
         */
        URI* getLocalURI() { return _localURI; }

        /*
         * Method the send an instant text message to the connected peer
         * The connection has to be established, ie connectionState = CONNECTION_STATE_CONNECTED
         *
         * @param message	The text message to send. //TODO be able to send message with spaces in it
         * @return int	0 on success
         *               1 otherwise
         */ 
        int sendInstantMessage( std::string message );

        /*
         * Free memory allocated by the pjsip library
         *
         * @return int	0 on success
         *              1 otherwise
         */
        int pjsip_shutdown();

        /*
         * Return a string form of the session media
         */
        std::string mediaToString( void );

        /*
         * Return the connection state string value
         */
        std::string getConnectionStateStr( connectionState state );

        /*
         * Return the connection state 
         */
        connectionState getConnectionState( void );

        /*
         * Change the invite answer mode. In AUTO mode, any new invite session is automatically accepted 
         * In manual mode, the user agent server can accept or refuse the incoming call
         */
        void setInviteAutoAnswer( bool mode );

        /*
         * Test if the user has an incoming call  
         *
         * @return bool	true if a call is incoming
         *               false otherwise
         */
        bool hasIncomingCall( void );

    private:
        /*
         * The SIP module name
         */
        std::string _name;

        /*
         * The local SIP address
         */
        URI * _localURI;

        /*
         * Initialize the pjsip_module structure
         */
        void init_sip_module( void );

        UserAgent(const UserAgent&); //No Copy Constructor
        UserAgent& operator=(const UserAgent&); //No Assignment Operator
};

#endif // _USER_AGENT_H
