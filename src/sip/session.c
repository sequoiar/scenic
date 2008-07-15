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

#include "session.h"

#include <stdlib.h>

struct _session* _sipsession;

void 
session_init( void ){
	_sipsession = (struct _session*)malloc(sizeof( struct _session ));
	// Default port
	_sipsession->type = SIP_SESSION;
	_sipsession->local_port = DEFAULT_SIP_PORT;
}

int 
session_connect( void ){
	return 0;
}

int 
session_disconnect( void ){
	free( _sipsession );
	return 0;
}


int main( int argc, char** argv ){
	return 0;
}
