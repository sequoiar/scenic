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

SIPSession::SIPSession() : Session( SIP ){

	app_ua =  new UserAgent();

}

SIPSession::~SIPSession(){}

int SIPSession::session_connect( int r_port, std::string r_ip ){
	this->init( r_port, r_ip );
	return 0;
}

int SIPSession::session_disconnect( void ){
	return 0;
}

void SIPSession::init( int r_port, std::string r_ip ) {
	app_ua->init_pjsip_modules();
}

