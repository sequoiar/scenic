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

#define THIS_FILE		"useragent.cpp"
#define APP_NAME		"miville"
#define DEFAULT_SIP_PORT	5060

#include <pjsip.h>
#include <pjlib.h>
#include <pjsip_ua.h>
#include <pjlib-util.h>

#include <string>

class UserAgent {

	public:
		UserAgent();
		~UserAgent();

		int init_pjsip_modules( void );

		int build_invite_request( std::string uri, std::string callerid, int port );
	private:
		std::string name;				/* Module name */
		std::string hostname;				/* Module hostname */
		std::string address;				/* Local Ip address */
		int port;					/* SIP port */		

		void init_sip_module( void );

};

#endif // _USER_AGENT_H
