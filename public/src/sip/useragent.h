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

#define THIS_FILE               "useragent"
#define _LOCAL_IP_ADDRESS       "127.0.0.1"
#define DEFAULT_SIP_PORT        5060
#define PJ_LOG_LEVEL            2

/* @file	useragent.h
 * @brief	A SIP useragent. 
 * Implements the SIP stacks from the transaction layer to the transport layer as described in RFC 3261.
 */

#include <pjsip.h>
#include <pjlib.h>
#include <pjsip_ua.h>
#include <pjlib-util.h>
#include <string>
#include <boost/python.hpp>

#include "sdp.h"
#include "uri.h"
#include "instantmessaging.h"

/*
 * Enumerate the different connection states
 */
enum connectionState {
    // The user agent is not initialized
    CONNECTION_STATE_NULL,
    // After the initialization, the user agent is ready to accept incoming connections
    // and to make calls
    CONNECTION_STATE_READY,
    // Transitory state. A connection try is pending
    CONNECTION_STATE_CONNECTING,
    // An incoming invite request has been received
    CONNECTION_STATE_INCOMING,
    // The connection is established between the two peers
    CONNECTION_STATE_CONNECTED,
    // The connection has been properly terminated
    CONNECTION_STATE_DISCONNECTED,
    // The connection failed because of a time out error. The host was probably unreachable
    CONNECTION_STATE_TIMEOUT,
    // The connection failed because no compatible media could have been found between the two peers
    CONNECTION_STATE_NOT_ACCEPTABLE,
    // An incoming text message
    CONNECTION_STATE_INCOMING_MESSAGE
};

/*
 * Enumerate the two answer modes, ie the behaviour of the user agent when receiving an 
 * invite request
 */
enum answerMode {
    // The user agent will automatically handle the request as if it was accepted
    ANSWER_MODE_AUTO,
    // The user is notified of an incoming invite request and he has to be choose 
    // betwwen accepting or refusing the call
    ANSWER_MODE_MANUAL
};

/*
 * Enumerate the different error cases
 */
enum errorCode {
    // Everything went fine
    NO_ERROR,
    // Try to initialize the session as it was already done
    ERROR_INIT_ALREADY_DONE,
    // Try to shutdown the session as it was already done
    ERROR_SHUTDOWN_ALREADY_DONE,
    // Connection timeout error because of an unreachable host
    ERROR_HOST_UNREACHABLE,
    // SDP negociation error; no compatible media have been found during the negociation
    ERROR_NO_COMPATIBLE_MEDIA,
    // Try to build an invite request as the user agent was not in the right state
    ERROR_CONNECTION_NOT_READY,
    // Not connected as it had to be 
    ERROR_NOT_CONNECTED
};

// Some typedef to lighten the code writing
typedef enum connectionState connectionState;
typedef enum answerMode amswerMode;
typedef enum errorCode errorCode;

/*
 * Second thread entry point.
 * This thread will take care of listen to sip events and process to incoming messages 
 * through the pjsip callbacks
 */

extern int startThread( void *arg);

/*
 * Callback from C++ to python when the connection state changes
 *
 * @param state The new state of the connection
 */
extern void py_connection_callback( std::string conn_state );

class UserAgent
{
    public:
        /*
         * Create a new UserAgent object
         * @param	name	The application name
         * @param   port    The SIP listening port
         */
        UserAgent( std::string name, int port );

        /*
         * Class destructor
         */
        ~UserAgent();

        /*
         * Initialize all the mandatory data structures from the PJSIP library
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
         * @param dir   The stream direction. Default value: sendrecv (bidirectional)
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
        int send_instant_message( std::string message );

        std::string get_message( void );

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
         * Return the error string reason
         */
        std::string getErrorReason( errorCode code );

        /*
         * Return the error code
         */
        errorCode getErrorCode( void );


        /*
         * Change the invite answer mode. In AUTO mode, any new invite session is automatically accepted
         * In manual mode, the user agent server can accept or refuse the incoming call
         * 
         * @param mode  The answer mode. Default value: 'auto'
         */
        void setAnswerMode( int mode );

        /*
         * Read acccessor. Get the answer mode
         */
        std::string getAnswerMode( void );

        /*
         * Test if the user has an incoming call
         *
         * @return bool	true if a call is incoming
         *               false otherwise
         */
        bool hasIncomingCall( void );

        /*
         * Set the python runnning instance as an PyObject.
         * As the C++ module is asynchronous, we need the C++ to call back the python module.
         * To callback on the right instance, we need that python gives us a memory reference
         * of its instance
         * basically, the python calls the C++ module, which will call later (when the result is available) 
         * the same python.
         *
         * @param p The python instance as a transparent PyObject 
         */
        void set_python_instance(PyObject *p);


        /*
         * Initialize and acquire the python global interpreter lock. Should be called in the main thread 
         * before any operation engaging in any thread operation.
         */
        void initPython();

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
        
        /*
         * Restore the connection state to the CONNECTION_STATE_READY
         */
        void connection_prepare( void );

        /*
         * If we need to switch to a synchronous behaviour
         *
         * @param state The current state. The function returns when the current state changed
         */
        void wait_for_response( connectionState state );

        UserAgent(const UserAgent&); //No Copy Constructor
        UserAgent& operator=(const UserAgent&); //No Assignment Operator
};


#endif // _USER_AGENT_H
