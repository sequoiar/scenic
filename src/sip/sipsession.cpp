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

SIPSession::SIPSession( int port )
    : Session( SIP, port ), _app_ua( new UserAgent( _APP_NAME, port ) ){
    //_app_ua =  new UserAgent( _APP_NAME, port );
    _app_ua->init_pjsip_modules( );
}


SIPSession::~SIPSession(){}

int SIPSession::connect( std::string r_uri, int r_port ){
    _app_ua->create_invite_session( r_uri, r_port );
    return 0;
}


int SIPSession::disconnect( void ){
    return 0;
}


int SIPSession::accept( void ){
    return 0;
}


int SIPSession::refuse( int reason ){
    return 0;
}


void SIPSession::build_sdp( void ){}


void SIPSession::startMainloop( void ){
    _app_ua->listen();
}


