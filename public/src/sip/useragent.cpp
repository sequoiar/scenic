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
#include <iostream>


/**************** STATIC VARIABLES AND FUNCTIONS (callbacks) **************************/

static void getRemoteSDPFromOffer( pjsip_rx_data *rdata, pjmedia_sdp_session** r_sdp );

// Documentated from the PJSIP Developer's Guide, available on the pjsip website

/*
 * Session callback
 * Called when the invite session state has changed.
 *
 * @param	inv	A pointer on a pjsip_inv_session structure
 * @param	e	A pointer on a pjsip_event structure
 */
static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e );

/*
 * Called when the invote usage module has created a new dialog and invite
 * because of forked outgoing request.
 *
 * @param	inv	A pointer on a pjsip_inv_session structure
 * @param	e	A pointer on a pjsip_event structure
 */
static void call_on_forked( pjsip_inv_session *inv, pjsip_event *e ){}

/*
 * Session callback
 * Called after SDP offer/answer session has completed.
 *
 * @param	inv	A pointer on a pjsip_inv_session structure
 * @param	status	A pj_status_t structure
 */
static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status );

/*
 * Session callback
 * Called whenever any transactions within the session has changed their state.
 * Useful to monitor the progress of an outgoing request.
 *
 * @param	inv	A pointer on a pjsip_inv_session structure
 * @param	tsx	A pointer on a pjsip_transaction structure
 * @param	e	A pointer on a pjsip_event structure
 */
static void call_on_tsx_state_changed( pjsip_inv_session *inv, pjsip_transaction *tsx,
                                       pjsip_event *e );

/*
 * Called to handle incoming requests outside dialogs
 * @param   rdata
 * @return  pj_bool_t
 */
static pj_bool_t on_rx_request( pjsip_rx_data *rdata );

/*
 * Called to handle incoming response
 * @param	rdata
 * @return	pj_bool_t
 */
static pj_bool_t on_rx_response( pjsip_rx_data *rdata );

/*
 * Session callback
 * Called whenever the invite session has received new offer from peer.
 * It will not send outgoing message. It just keep the answer for SDP negociation process.
 *
 * @param	inv	A pointer on a pjsip_inv_session structure
 * @param	offer	A constant pointer on a pjmedia_sdp_session structure
 */
static void on_rx_offer( pjsip_inv_session *inv, const pjmedia_sdp_session *offer );

/*
 *	The SIP endpoint
 */
static pjsip_endpoint *endpt;

/*
 *  The global pool factory
 */
static pj_caching_pool c_pool;

/*
 *	The SIP module
 */
static pjsip_module mod_ua;

/*
 *  The invite session
 */
pjsip_inv_session* inv_session;

/*
 *  A bool to indicate whether or not the connection is up
 */
static pj_bool_t complete;

/*
 *  The local Session Description Protocol body
 */
static Sdp *local_sdp;

/*************************************************************************************************/


UserAgent::UserAgent( std::string name, int port )
    : _name(name), _localURI(0)
{

    // Use the empty constructor, so that the URI could guess the local parameters
   _localURI = new URI( port );

}


UserAgent::~UserAgent(){}

void UserAgent::init_sip_module( void ){
    mod_ua.name = pj_str((char*)this->_name.c_str());
    mod_ua.id = -1;
    mod_ua.priority = PJSIP_MOD_PRIORITY_APPLICATION;
    mod_ua.on_rx_request = &on_rx_request;
    mod_ua.on_rx_response = &on_rx_response;
}


int UserAgent::init_pjsip_modules(  ){

    pj_status_t status;
    pj_sockaddr local;
    pj_uint16_t listeningPort;

    // Init SIP module
    init_sip_module();

    // Init the pj library. Must be called before using the library
    status = pj_init();
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Init the pjlib-util library.
    status = pjlib_util_init();
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Create a pool factory to allocate memory
    pj_caching_pool_init( &c_pool, &pj_pool_factory_default_policy, 0 );

    /* Create the endpoint */
    status = pjsip_endpt_create( &c_pool.factory, NULL, &endpt );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Add UDP Transport */
    listeningPort = (pj_uint16_t) getLocalURI()->getPort();
    pj_sockaddr_init( pj_AF_INET(), &local, NULL, listeningPort ); 
    status = pjsip_udp_transport_start( endpt, &local.ipv4 , NULL, 1, NULL );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Create transaction layer */
    status = pjsip_tsx_layer_init_module( endpt );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Initialize transaction user layer */
    status = pjsip_ua_init_module( endpt, NULL );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Init the callbacks for INVITE session */
    pjsip_inv_callback inv_cb;
    pj_bzero( &inv_cb, sizeof(inv_cb) );
    inv_cb.on_state_changed = &call_on_state_changed;
    inv_cb.on_new_session = &call_on_forked;
    inv_cb.on_media_update = &call_on_media_update;
    inv_cb.on_tsx_state_changed = &call_on_tsx_state_changed;
    inv_cb.on_rx_offer = &on_rx_offer;

    /* Initialize invite session module */
    status = pjsip_inv_usage_init( endpt, &inv_cb );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Initialize 100rel support */
    status = pjsip_100rel_init_module( endpt );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Register the module to receive incoming requests */
    status = pjsip_endpt_register_module( endpt, &mod_ua );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Initialize the SDP class instance */
    int aport = 12488; // random TODO Get the transport ports from gstreamer
    int vport = 12490; // random TODO Get the transport ports from gstreamer
    local_sdp = new Sdp( getLocalURI()->getHostIP(), aport, vport );

    PJ_LOG(3, (THIS_FILE, "Ready to accept incoming calls..."));

    return 1;
}


int UserAgent::create_invite_session( std::string uri ){

    pjsip_dialog *dialog;
    pjsip_tx_data *tdata;
    pj_status_t status;
    URI *remote, *local;
    pj_str_t from, to;
    char tmp[90];
    
    remote = new URI( uri );
    local = getLocalURI();

    pj_ansi_sprintf( tmp, remote->getAddress().c_str() );
    from = pj_str(tmp);

    to.ptr = (char*) remote->getAddress().c_str() ;
    to.slen = remote->getAddress().length();

    status = pjsip_dlg_create_uac( pjsip_ua_instance(), &from,
                                  NULL, 
                                  &to,
                                  NULL,
                                   &dialog );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Building the local SDP offer
    local_sdp->createInitialOffer( dialog->pool );

    status = pjsip_inv_create_uac( dialog, local_sdp->getLocalSDPSession(), 0, &inv_session );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    status = pjsip_inv_invite( inv_session, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Start the mainloop
    listen();

    return PJ_SUCCESS;
}


void UserAgent::listen( void){
    for(; !complete;) {
        pj_time_val timeout = {0, 10};
        pjsip_endpt_handle_events( endpt, &timeout );
    }
}


void UserAgent::addMediaToSession( int mime_type, std::string codecs ){
    local_sdp->addMediaToSDP( mime_type, codecs );    
}

static void getRemoteSDPFromOffer( pjsip_rx_data *rdata, pjmedia_sdp_session** r_sdp ){
    pjmedia_sdp_session *sdp;
    pjsip_msg *msg;
    pjsip_msg_body *body;

    msg = rdata->msg_info.msg;
    body = msg->body;

    pjmedia_sdp_parse( rdata->tp_info.pool, (char*)body->data, body->len, &sdp );

    *r_sdp = sdp;
}


/********************** Callbacks Implementation **********************************/

static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e ){
    if( inv->state == PJSIP_INV_STATE_DISCONNECTED )
        complete = 1;
}


static pj_bool_t on_rx_request( pjsip_rx_data *rdata ){
    
    pj_status_t status;
    pj_str_t reason;
    unsigned options = 0;
    pjsip_dialog* dialog;
    pjsip_tx_data *tdata;
    pjmedia_sdp_session *r_sdp;

    /* Respond statelessly any non-INVITE requests with 500 */
    if( rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD ) {
        if( rdata->msg_info.msg->line.req.method.id != PJSIP_ACK_METHOD ) {
            reason = pj_str((char*)" user agent unable to handle this request ");
            pjsip_endpt_respond_stateless( endpt, rdata, 500, &reason, NULL, NULL );
            return PJ_TRUE;
        }
    }
    // Verify that we can handle the request
    status = pjsip_inv_verify_request( rdata, &options, NULL, NULL, endpt, NULL );
    if( status != PJ_SUCCESS ){
        reason = pj_str((char*)" user agent unable to handle this INVITE ");
        pjsip_endpt_respond_stateless( endpt, rdata, 405, &reason, NULL, NULL );
        return PJ_TRUE;
    }
    // Have to do some stuff here with the SDP
    // We retrieve the remote sdp offer in the rdata struct to begin the negociation
    getRemoteSDPFromOffer( rdata, &r_sdp );
    local_sdp->receivingInitialOffer( rdata->tp_info.pool, r_sdp );

    /* Create the local dialog (UAS) */
    //pj_str_t contact = pj_str((char*) "<sip:test@127.0.0.1:5060>");
    status = pjsip_dlg_create_uas( pjsip_ua_instance(), rdata, NULL, &dialog );
    if( status != PJ_SUCCESS ) {
        pjsip_endpt_respond_stateless( endpt, rdata, 500, &reason, NULL, NULL );
        return PJ_TRUE;
    }
    // Specify media capability during invite session creation
    status = pjsip_inv_create_uas( dialog, rdata, local_sdp->getLocalSDPSession(), 0, &inv_session );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Send a 180/Ringing response
    status = pjsip_inv_initial_answer( inv_session, rdata, 180, NULL, NULL, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    //Create and send a 200 response
    status = pjsip_inv_answer( inv_session, 200, NULL, NULL, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Done */

    return PJ_SUCCESS;
}


static pj_bool_t on_rx_response( pjsip_rx_data *rdata ){
    /* Respond statelessly any non-INVITE requests with 500 */
    //if( rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD ) {
    printf("on_rx_response\n");
    return PJ_SUCCESS;
}


static void call_on_tsx_state_changed( pjsip_inv_session *inv, pjsip_transaction *tsx,
                                       pjsip_event *e ){}


static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status ){
    // We need to get the final media choice and send it to gstreamer
    // Maybe we want to start the data streaming now...

    if( status != PJ_SUCCESS )
        return;

    const pjmedia_sdp_session *r_sdp;
    pjmedia_sdp_media *media;
    int c_count;

    // Get local and remote SDP
    pjmedia_sdp_neg_get_active_remote( inv->neg, &r_sdp );

    // Retrieve the codecs
    media = r_sdp->media[0];
    c_count = media->desc.fmt_count;
    //printf("Codec count: %i\n" , c_count);
    //printf("Codec payload: %s\n" , media->desc.fmt[c_count-1].ptr);
}


static void on_rx_offer( pjsip_inv_session *inv, const pjmedia_sdp_session *offer ){
    printf("Invite session received new offer from peer - %s\n", offer->name.ptr);
}


