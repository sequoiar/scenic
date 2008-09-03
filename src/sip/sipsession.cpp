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
    pj_status_t status;

    _app_ua = new UserAgent( APP_NAME, port );
    // Init the pjsip library modules
    status = _app_ua->init_pjsip_modules( );
}


SIPSession::SIPSession( SIPSession const& )
    : Session( PROTOCOL_SIP,
               DEFAULT_SIP_PORT ), _app_ua( NULL ){
    _app_ua = new UserAgent( APP_NAME, DEFAULT_SIP_PORT );
    // Init the pjsip library modules
    _app_ua->init_pjsip_modules( );
}


SIPSession::~SIPSession(){}

int SIPSession::connect( std::string r_uri ){
    return _app_ua->inv_session_create( r_uri );
}


int SIPSession::disconnect( void ){
    return _app_ua->inv_session_end();
}


int SIPSession::shutdown( void ){
    return _app_ua->pjsip_shutdown();
}


int SIPSession::reinvite( void ){
    return _app_ua->inv_session_reinvite();
}


int SIPSession::sendInstantMessage( std::string msg ){
    return _app_ua->sendInstantMessage( msg );
}


int SIPSession::accept( void ){
    return _app_ua->inv_session_accept();
}


int SIPSession::refuse( void ){
    return _app_ua->inv_session_refuse();
}


void SIPSession::build_sdp( void ){}

void SIPSession::setMedia( std::string type, std::string codecs, int port, std::string dir ){
    _app_ua->setSessionMedia( type, codecs, port, dir );
}

std::string SIPSession::getConnectionState( void ){
    return _app_ua->getConnectionStateStr( _app_ua->getConnectionState() );
}


std::string SIPSession::mediaToString( void ){
    return _app_ua->mediaToString();
}


bool SIPSession::incomingInvite( void ){
    return _app_ua->hasIncomingCall();
}

void SIPSession::setAutoAnswer( bool mode ){
    _app_ua->setInviteAutoAnswer( mode );
}
