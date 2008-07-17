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

#include <pjsip.h>
#include <pjlib.h>
#include <pjsip_ua.h>
#include <pjlib-util.h>

#include <string>

//#include "sipsession.h"
#include "useragent.h"
//#include "sip_sdp.h"

#define SIP	0

class Session {
	public:
		/* Class constructor */
		Session( int type ) { _protocol = type; }

		/* Virtual class destructor */
		~Session() {}

		/* API method */
		virtual int session_connect( int r_port, std::string r_ip ) = 0;

		/* API method */
		virtual int session_disconnect( ) = 0;

	private:
		int _protocol;
};


#endif // _SESSION_H
