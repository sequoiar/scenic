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
 * @file 	session.h
 * @brief	An interface to expose to the core the available methods
 */

#include <string>
#include <pjsip.h>
#include <pjlib.h>
#include <pjsip_ua.h>
#include <pjlib-util.h>

#include "useragent.h"

#define SIP	0

class Session {
	public:
		/* 
		 * Class construct. Create a new Session object.
		 * @param type	The protocol type. As for now, only SIP (Session Initiation Protocol) is available
		 */
		Session( int type ) { _protocol = type; }

		/* 
		 * Class destructor 
		 */
		~Session() {}

		/* 
		 * Establishes a connection with a remote host.
		 * @param 	r_port	The remote connection port
		 * @param 	r_uri	The remote SIP address 
		 */
		virtual int connect( int r_port, std::string r_uri ) = 0;

		/*
		 * Terminate a current remote connection
		 */
		virtual int disconnect( ) = 0;

		/*
		 * Accept incoming request 
		 */
		virtual int accept( void ) = 0;

		virtual int refuse( int reason ) = 0;

		virtual void build_sdp() = 0;

	private:
		/* The communication protocol */
		int _protocol;
};


#endif // _SESSION_H
