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
 * @brief	A class to manage sessions
 */

#include <pjsip.h>
#include <pjlib.h>
#include <pjsip_ua.h>
#include <pjlib-util.h>

#define SIP_SESSION		0
#define DEFAULT_SIP_PORT	5060

struct _session{
	char* local_ip;
	char* remote_ip;
	int local_port;
	int remote_port;
};

int session_init( void );

int session_connect( int r_port, char* r_ip );

int session_disconnect( );

#endif // _SESSION_H
