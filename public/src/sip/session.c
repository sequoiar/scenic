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

static pj_bool_t	g_complete;
static pjsip_endpoint*	g_endpt;			/* SIP endpoint */
static pj_caching_pool	caching_pool;			/* Global pool factory */

/* Called when invite session changed */
static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e ){}
/* Called when dialog has forked */
static void call_on_forked( pjsip_inv_session *inv, pjsip_event *e ){}
/* Called when SDP negociation is done in the call */
static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status ){}
/* Called to handle incoming requests outside dialogs */
static pj_bool_t on_rx_request( pjsip_rx_data *rdata ){}

/* The SIP module */
static pjsip_module mod_ua = 
{
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
};

#define THIS_FILE	"session.c"

int 
session_init( void ){

	_sipsession = (struct _session*)malloc(sizeof( struct _session ));
	pj_status_t status;
	
	// Default port
	_sipsession->local_port = DEFAULT_SIP_PORT;

	// Init the pj library. Must be called before using the library
	status = pj_init();
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	// Init the pjlib-util library.
	status = pjlib_util_init();
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	// Create a pool factory to allocate memory
	pj_caching_pool_init( &caching_pool, &pj_pool_factory_default_policy, 0 );

	//const pj_str_t* hostname;
	const char* endpt_name = pj_gethostname()->ptr;

	/* Create the endpoint */
	status = pjsip_endpt_create( &caching_pool.factory, endpt_name, &g_endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Add UDP Transport */
	pj_sockaddr_in addr;
	pjsip_host_port addrname;
	pj_bzero( &addr, sizeof(addr));
	addr.sin_family = PJ_AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = pj_htons((pj_uint16_t)DEFAULT_SIP_PORT);
	addrname.host = pj_str("192.168.1.204");
	addrname.port = 5060;
	status = pjsip_udp_transport_start( g_endpt, &addr, &addrname, 1, NULL );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	/* Create transaction layer */
	status = pjsip_tsx_layer_init_module( g_endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Initialize transaction user layer */
	status = pjsip_ua_init_module( g_endpt, NULL );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Init the callbacks for INVITE session */
	pjsip_inv_callback inv_cb;
	pj_bzero( &inv_cb, sizeof(inv_cb) );
	inv_cb.on_state_changed = &call_on_state_changed;
	inv_cb.on_new_session = &call_on_forked;
	inv_cb.on_media_update = &call_on_media_update;

	/* Initialize invite session module */
	status = pjsip_inv_usage_init( g_endpt, &inv_cb );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Initialize 100rel support */
	status = pjsip_100rel_init_module( g_endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Register the module to receive incoming requests */
	status = pjsip_endpt_register_module( g_endpt, &mod_ua );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	PJ_LOG(3,(THIS_FILE, "Ready to accept incoming calls..."));

	session_connect( 88884 , "127.0.0.1");
	for(; !g_complete;) {
		pj_time_val timeout = {0, 10};
		pjsip_endpt_handle_events( g_endpt, &timeout );
	}

	//dump_pool_usage(THIS_FILE, &caching_pool);

}

int 
session_connect( int r_port, char* r_ip ){
	

	pj_str_t target = pj_str("sip:136@69.157.207.98:5060");
	pj_str_t from = pj_str("\"Manu\" <sip:localuser@192.168.1.204:5061>");
	pj_str_t to = pj_str("\"Remote\" <sip:136@69.157.207.98:5060>");
	pj_str_t contact = from;
	pj_status_t status;
	pjsip_tx_data* request;
	pj_str_t body = pj_str("toto");
	pjsip_method method;

	pjsip_method_set( &method, PJSIP_INVITE_METHOD );
	status = pjsip_endpt_create_request( g_endpt, &method, &target, &from, &to, &contact, NULL, -1, &body, &request );
	assert( status == PJ_SUCCESS );
	status = pjsip_endpt_send_request_stateless( g_endpt, request, NULL, NULL );
	assert( status == PJ_SUCCESS );

	return 0;
}

int 
session_disconnect( void ){
	free( _sipsession );
	return 0;
}



int main( int argc, char** argv ){
	session_init();
	//session_connect( 5060 , "127.0.0.1");
	return 0;
}
