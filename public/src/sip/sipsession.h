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
 * @file 	sipsession.h
 * @brief	The SIP implementation
 */

#include <string>

#include "session.h"
#include "useragent.h"

#define _APP_NAME	"miville"

class SIPSession : public Session {

	public:
		/*
		 * Create a new SIP session object
		 */
		SIPSession();
		
		/*
		 * Class destructor
		 */
		~SIPSession();
		
		/* 
		 * Establishes a SIP connection with a remote host.
		 * @param 	r_port	The remote connection port (default SIP port: 5060)
		 * @param 	r_ip	The remote IP address 
		 */
		int session_connect( int r_port, std::string r_ip );
		
		/*
		 * Terminate a current SIP remote connection
		 */
		int session_disconnect();

		int accept( void );

		int refuse( int reason );

		void build_sdp( void );

	private:
		/*
		 * Initialize a SIP connection.
		 */
		void init( );

		/* The application SIP User Agent */
		UserAgent* _app_ua;
};

#endif // _SIP_SESSION_H
