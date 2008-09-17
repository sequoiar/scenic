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

#include "sipsession.h"
#include <iostream>

SIPSession::SIPSession( int port )
    : Session( PROTOCOL_SIP, port ), _app_ua(NULL) {
    // Set the session port
    set_session_port(port);
    // Create the user agent module
    _app_ua = new UserAgent( APP_NAME, port );
    // Initialize the user agent module
    init( );
}


SIPSession::SIPSession( SIPSession const& )
    : Session( PROTOCOL_SIP,
               DEFAULT_SIP_PORT ), _app_ua( NULL ){
    // Set the session port
    set_session_port( DEFAULT_SIP_PORT );
    // Create the user agent module
    _app_ua = new UserAgent( APP_NAME, DEFAULT_SIP_PORT );
    // Initialize the user agent module
    init();
}


SIPSession::~SIPSession(){
    // Delete the pointer reference on the user agent module
    delete _app_ua; _app_ua = 0;
}


int SIPSession::connect( std::string r_uri ){
    // tells the user agent module to create a connection 
    return _app_ua->inv_session_create( r_uri );
}


int SIPSession::disconnect( void ){
    // tells the user agent module to end the current connection 
    return _app_ua->inv_session_end();
}


int SIPSession::shutdown( void ){
    // tells the user agent module to free the allocated memory and to shutdown pjsip 
    return _app_ua->pjsip_shutdown();
}


int SIPSession::init( void ) {
    // tells the user agent module to initialize its components
    return _app_ua->init_pjsip_modules();
}


int SIPSession::reinvite( void ){
    // tells the user agent module to update the current connection 
    return _app_ua->inv_session_reinvite();
}


int SIPSession::send_instant_message( std::string msg ){
    // tells the user agent module to send the text message 
    return _app_ua->send_instant_message( msg );
}


int SIPSession::accept( void ){
    // tells the user agent module to accept the incoming connection 
    return _app_ua->inv_session_accept();
}


int SIPSession::refuse( void ){
    // tells the user agent module to refuse the incoming connection 
    return _app_ua->inv_session_refuse();
}

void SIPSession::set_media( std::string type, std::string codecs, int port, std::string dir ){
    _app_ua->setSessionMedia( type, codecs, port, dir );
}


std::string SIPSession::get_connection_state( void ){
    // Return the equivalent string form of the current connection state
    return _app_ua->getConnectionStateStr( _app_ua->getConnectionState() );
}


std::string SIPSession::get_error_reason( void ){
    // Return the equivalent string form of the current error status 
    return _app_ua->getErrorReason( _app_ua->getErrorCode() );
}


std::string SIPSession::media_to_string( void ){
    return _app_ua->mediaToString();
}


void SIPSession::set_answer_mode( int mode ){
    _app_ua->setAnswerMode( mode );
}


std::string SIPSession::get_answer_mode( void ){
    return _app_ua->getAnswerMode();
}

void SIPSession::set_python_instance(PyObject *p ){
    _app_ua->set_python_instance(p);
}
