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

#include "useragent.h"

#include <stdlib.h>

/* Called when invite session changed */
static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e ){}

/* Called when dialog has forked */
static void call_on_forked( pjsip_inv_session *inv, pjsip_event *e ){}

/* Called when SDP negociation is done in the call */
static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status ){}

/* Called to handle incoming requests outside dialogs */
//static pj_bool_t on_rx_request( pjsip_rx_data* d ){}

UserAgent::UserAgent(){}

UserAgent::~UserAgent(){}

int UserAgent::init_pjsip_modules( void ){

	pj_status_t status;

	this->name = APP_NAME; 
	/*pj_app_var->mod_ua = {
		NULL, NULL,
		{"miville", 11},
		-1,
		PJSIP_MOD_PRIORITY_APPLICATION,
		NULL,
		NULL,
		NULL,
		NULL,
		&on_rx_request,
		NULL,
		NULL,
		NULL,
		NULL,
	};*/


	// Init the pj library. Must be called before using the library
	status = pj_init();
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	// Init the pjlib-util library.
	status = pjlib_util_init();
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	// Create a pool factory to allocate memory
	pj_caching_pool_init( &this->c_pool, &pj_pool_factory_default_policy, 0 );

	/* Create the endpoint */
	//this->hostname = pj_gethostname()->ptr;
	status = pjsip_endpt_create( &this->c_pool.factory, pj_gethostname()->ptr, &this->endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Add UDP Transport */
	pj_sockaddr_in addr;
	pjsip_host_port addrname;
	pj_bzero( &addr, sizeof(addr));
	addr.sin_family = PJ_AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = pj_htons((pj_uint16_t)DEFAULT_SIP_PORT);
	addrname.host = pj_str((char*)("192.168.1.204"));
	addrname.port = 5060;
	status = pjsip_udp_transport_start( this->endpt, &addr, &addrname, 1, NULL );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	/* Create transaction layer */
	status = pjsip_tsx_layer_init_module( this->endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Initialize transaction user layer */
	status = pjsip_ua_init_module( this->endpt, NULL );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Init the callbacks for INVITE session */
	pjsip_inv_callback inv_cb;
	pj_bzero( &inv_cb, sizeof(inv_cb) );
	inv_cb.on_state_changed = &call_on_state_changed;
	inv_cb.on_new_session = &call_on_forked;
	inv_cb.on_media_update = &call_on_media_update;

	/* Initialize invite session module */
	status = pjsip_inv_usage_init( this->endpt, &inv_cb );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Initialize 100rel support */
	status = pjsip_100rel_init_module( this->endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Register the module to receive incoming requests */
	//status = pjsip_endpt_register_module( this->endpt, &mod_ua );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	PJ_LOG(3,(THIS_FILE, "Ready to accept incoming calls..."));

	//make_call();
	//session_connect( 88884 , "127.0.0.1");
	for(; !this->complete;) {
		pj_time_val timeout = {0, 10};
		pjsip_endpt_handle_events( this->endpt, &timeout );
	}

    return 0;
}

