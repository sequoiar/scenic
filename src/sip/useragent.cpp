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
static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e );

/* Called when dialog has forked */
static void call_on_forked( pjsip_inv_session *inv, pjsip_event *e ){}

/* Called when SDP negociation is done in the call */
static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status );

static void call_on_tsx_state_changed( pjsip_inv_session *inv, pjsip_transaction *tsx, pjsip_event *e );

/* Called to handle incoming requests outside dialogs */
static pj_bool_t on_rx_request( pjsip_rx_data *rdata );
static pj_bool_t on_rx_response( pjsip_rx_data *rdata );

static pjsip_endpoint *endpt;				/* SIP endpoint */
static pj_caching_pool c_pool;				/* Global pool factory */
static pjsip_module mod_ua;				/* The SIP module */
static pj_bool_t complete;

UserAgent::UserAgent(){}

UserAgent::~UserAgent(){}

void
UserAgent::init_sip_module( void ){
	
	mod_ua.name = pj_str((char*)this->name.c_str());
	mod_ua.id = -1;
	mod_ua.priority = PJSIP_MOD_PRIORITY_APPLICATION;
	mod_ua.on_rx_request = &on_rx_request;
	mod_ua.on_rx_response = &on_rx_response;

}



int UserAgent::init_pjsip_modules( void ){

	pj_status_t status;

	this->name = APP_NAME; 

	// Init SIP module
	init_sip_module();

	// Init the pj library. Must be called before using the library
	status = pj_init();
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	// Init the pjlib-util library.
	status = pjlib_util_init();
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	// Create a pool factory to allocate memory
	pj_caching_pool_init( &c_pool, &pj_pool_factory_default_policy, 0 );

	/* Create the endpoint */
	status = pjsip_endpt_create( &c_pool.factory, pj_gethostname()->ptr, &endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Add UDP Transport */
	std::string addr_ip = "192.168.1.204";
	pj_sockaddr_in addr;
	pjsip_host_port addrname;
	pj_bzero( &addr, sizeof(addr));
	addr.sin_family = PJ_AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = pj_htons((pj_uint16_t)DEFAULT_SIP_PORT);
<<<<<<< HEAD:trunk/public/src/sip/useragent.cpp
	addrname.host = pj_str((char*)("192.168.1.204"));
=======
	addrname.host = pj_str((char*)addr_ip.c_str());
>>>>>>> switch pjsip-related variables to static attributes:trunk/public/src/sip/useragent.cpp
	addrname.port = 5060;
	status = pjsip_udp_transport_start( endpt, &addr, &addrname, 1, NULL );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	/* Create transaction layer */
	status = pjsip_tsx_layer_init_module( endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Initialize transaction user layer */
	status = pjsip_ua_init_module( endpt, NULL );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Init the callbacks for INVITE session */
	pjsip_inv_callback inv_cb;
	pj_bzero( &inv_cb, sizeof(inv_cb) );
	inv_cb.on_state_changed = &call_on_state_changed;
	inv_cb.on_new_session = &call_on_forked;
	inv_cb.on_media_update = &call_on_media_update;
	inv_cb.on_tsx_state_changed = &call_on_tsx_state_changed;

	/* Initialize invite session module */
	status = pjsip_inv_usage_init( endpt, &inv_cb );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Initialize 100rel support */
	status = pjsip_100rel_init_module( endpt );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	/* Register the module to receive incoming requests */
	status = pjsip_endpt_register_module( endpt, &mod_ua );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );
	
	PJ_LOG(3,(THIS_FILE, "Ready to accept incoming calls..."));

	build_invite_request("\"Test\" <sip:136@asterix.savoirfairelinux.net:5060>", "", -1);
	for(; !complete;) {
		pj_time_val timeout = {0, 10};
		pjsip_endpt_handle_events( endpt, &timeout );
	}

    return 0;
}

int 
UserAgent::build_invite_request( std::string uri, std::string callerid, int port ){

	pjsip_dialog *dialog;
	pjsip_inv_session* inv;
	pjsip_tx_data *tdata;
	pj_status_t status;
	std::string local = "\"Manu\" <sip:localuser@192.168.1.204:5060>";

	pj_str_t target = pj_str( (char*)uri.c_str() );
	pj_str_t from = pj_str( (char*) local.c_str() );
	pj_str_t to = pj_str( (char*)uri.c_str() );
	pj_str_t contact = from;

	status = pjsip_dlg_create_uac( pjsip_ua_instance(), &from, 
						&from,
						&to,
						&to,
						&dialog );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	status = pjsip_inv_create_uac( dialog, NULL, 0, &inv );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	status = pjsip_inv_invite( inv, &tdata );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	status = pjsip_inv_send_msg( inv, tdata );
	PJ_ASSERT_RETURN( status == PJ_SUCCESS , 1 );

	return PJ_SUCCESS;	

}


static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e ){
	
	if( inv->state == PJSIP_INV_STATE_DISCONNECTED ) 
		complete = 1;

}


static pj_bool_t on_rx_request( pjsip_rx_data *rdata ){

	/* Respond statelessly any non-INVITE requests with 500 */
	//if( rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD ) {
	printf("on_rx_request\n");
	return PJ_SUCCESS;


}

static pj_bool_t on_rx_response( pjsip_rx_data *rdata ){

	/* Respond statelessly any non-INVITE requests with 500 */
	//if( rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD ) {
	printf("on_rx_response\n");
	return PJ_SUCCESS;

}

static void call_on_tsx_state_changed( pjsip_inv_session *inv, pjsip_transaction *tsx, pjsip_event *e ){
	printf("Method receiced: %s\n", tsx->method.name );
}

static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status ){
	printf("SDP negociation done!\n");
}
