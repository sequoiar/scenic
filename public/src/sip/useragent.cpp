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

using std::cout;
using std::cerr;
using std::endl;


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
static void call_on_forked( pjsip_inv_session *inv, pjsip_event *e );

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
    status = pjsip_udp_transport_start( endpt, &local.ipv4, NULL, 1, NULL );
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
    char tmp[90], tmp1[90];

    remote = new URI( uri );
    local = getLocalURI();

    pj_ansi_sprintf( tmp, local->getAddress().c_str() );
    from = pj_str(tmp);

    pj_ansi_sprintf( tmp1, remote->getAddress().c_str() );
    to = pj_str(tmp1);

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


int UserAgent::terminate_invite_session( void ){
    pj_status_t status;
    pjsip_tx_data *tdata;

    status = pjsip_inv_end_session( inv_session, 404, NULL, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    //status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    return PJ_SUCCESS;
}


int UserAgent::sendInstantMessage( std::string msg ){
    pjsip_dialog *dlg;
    pj_str_t text;
    pj_status_t status;

    // Get the dialog instance
    dlg = inv_session->dlg;

    // Call the private method to do the job
    text = pj_str((char*)msg.c_str());
    status = send_im_dialog( dlg, &text );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    return PJ_SUCCESS;
}


void UserAgent::listen( void){
    for(; !complete;) {
        pj_time_val timeout = {0, 10};
        pjsip_endpt_handle_events( endpt, &timeout );
    }
}


void UserAgent::addMediaToSession( std::string codecs ){
    size_t pos;
    std::string media;
    int mime_type;

    // codecs looks like that: m=codec1/codec2/..../codecn/ where m = a for audio or v for video
    // We have got to retreive the media type first:
    pos = codecs.find("=", 0);
    media = codecs.substr(0, pos );
    codecs.erase(0, pos + 1);

    strcmp( media.c_str(),
            "a" ) == 0 ||
    strcmp( media.c_str(),
            "A" ) == 0 ? mime_type = MIME_TYPE_AUDIO :  mime_type = MIME_TYPE_VIDEO ;

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


pj_status_t UserAgent::send_im_dialog( pjsip_dialog *dlg, pj_str_t *msg ){
    pjsip_method msg_method;
    //pj_str_t name;
    const pj_str_t STR_TEXT =  pj_str((char*)"text");;
    const pj_str_t STR_PLAIN = pj_str((char*)"plain");
    pjsip_tx_data *tdata;
    pj_status_t status;

    //name = pj_str((char*)"MESSAGE");
    //name = { (char*)"MESSAGE" , 7};
    //msg_method->PJSIP_OTHER_METHOD, name };
    //msg_method = PJ_POOL_ZALLOC_T( dlg->pool , pjsip_method );
    msg_method.id = PJSIP_OTHER_METHOD;
    msg_method.name = pj_str((char*)"MESSAGE") ;


    // Must lock dialog
    pjsip_dlg_inc_lock( dlg );

    // Create the message request
    status = pjsip_dlg_create_request( dlg, &msg_method, -1 /*CSeq*/, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Attach "text/plain" body */
    tdata->msg->body = pjsip_msg_body_create( tdata->pool, &STR_TEXT, &STR_PLAIN, msg );

    // Send the request
    status = pjsip_dlg_send_request( dlg, tdata, -1, NULL);
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Done
    pjsip_dlg_dec_lock( dlg );

    return PJ_SUCCESS;
}


/********************** Callbacks Implementation **********************************/

static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e ){
    printf("Call state changed\n");
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
            pjsip_endpt_respond_stateless( endpt, rdata, MSG_METHOD_NOT_ALLOWED, &reason, NULL,
                                           NULL );
            return PJ_TRUE;
        }
    }
    // Verify that we can handle the request
    status = pjsip_inv_verify_request( rdata, &options, NULL, NULL, endpt, NULL );
    if( status != PJ_SUCCESS ){
        reason = pj_str((char*)" user agent unable to handle this INVITE ");
        pjsip_endpt_respond_stateless( endpt, rdata, MSG_METHOD_NOT_ALLOWED, &reason, NULL,
                                       NULL );
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
        pjsip_endpt_respond_stateless( endpt, rdata, MSG_SERVER_INTERNAL_ERROR, &reason, NULL,
                                       NULL );
        return PJ_TRUE;
    }
    // Specify media capability during invite session creation
    status = pjsip_inv_create_uas( dialog, rdata,
                                   local_sdp->getLocalSDPSession(), 0, &inv_session );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Send a 180/Ringing response
    status = pjsip_inv_initial_answer( inv_session, rdata, MSG_RINGING, NULL, NULL, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Check if the SDP negociation has been succesfully done
    status = local_sdp->startNegociation( rdata->tp_info.pool );

    if( status == PJ_SUCCESS ) {
        // Create and send a 200(OK) response
        status = pjsip_inv_answer( inv_session, MSG_OK, NULL, NULL, &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    }

    else{ // Negociation failed
          // Create and send a 488( Not acceptable here)
          // Probably no compatible media found in the remote SDP offer
        status = pjsip_inv_answer( inv_session, MSG_NOT_ACCEPTABLE_HERE, NULL, NULL, &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    }
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
                                       pjsip_event *e ){
    if( tsx->state == PJSIP_TSX_STATE_TERMINATED  &&
        tsx->role == PJSIP_ROLE_UAC ) {
        printf("stop loop\n");
        complete = 1;
    }

    else if( tsx->state == PJSIP_TSX_STATE_TRYING &&
             tsx->role == PJSIP_ROLE_UAS ){
        pjsip_rx_data *rdata;
        pjsip_msg *msg;
        pj_status_t status;

        // Incoming message request
        cout << "Request received" << endl;

        // Retrieve the body message
        rdata = e->body.tsx_state.src.rdata;
        msg = rdata->msg_info.msg;

        // Respond with OK message
        status = pjsip_dlg_respond( inv->dlg, rdata, MSG_OK, NULL, NULL, NULL );

        // Display the message
        cout << " ! New message : " << (char*)msg->body->data << endl;
    }
}


static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status ){
    // We need to get the final media choice and send it to gstreamer
    // Maybe we want to start the data streaming now...

    int nbMedia;
    int nbCodecs;
    int i, j;
    const pjmedia_sdp_session *r_sdp;
    pjmedia_sdp_media *media;

    if( status != PJ_SUCCESS )
        return;
    // Get local and remote SDP
    pjmedia_sdp_neg_get_active_local( inv->neg, &r_sdp );

    // Retrieve the media
    nbMedia = r_sdp->media_count;
    for( i=0; i<nbMedia ; i++ ){
        printf("Media %i: ", i);
        media = r_sdp->media[i];
        nbCodecs = media->desc.fmt_count;
        printf("Codec count: %i\n", nbCodecs);
        for( j=0 ; j<nbCodecs ; j++ ){
            printf("Codec payload: %s\n", media->desc.fmt[j].ptr);
        }
    }

    //TODO Call to the core to update the selected codecs
    // libboost
}


static void on_rx_offer( pjsip_inv_session *inv, const pjmedia_sdp_session *offer ){
    printf("Invite session received new offer from peer - %s\n", offer->name.ptr);
}


static void call_on_forked( pjsip_inv_session *inv, pjsip_event *e ){
    printf(
        "The invite session module has created a new dialog because of forked outgoing request\n");
}


