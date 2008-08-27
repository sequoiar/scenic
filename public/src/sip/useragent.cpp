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

#define RANDOM_SIP_PORT   rand() % 64000 + 1024

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
 * The pool to allocate memory
 */
static pj_pool_t *app_pool;

/*
 *	The SIP module
 */
static pjsip_module mod_ua;

/*
 *  The invite session
 */
pjsip_inv_session* inv_session;

/*
 * The connection state
 */
static connectionState _state;

/*
 * The connection state string
 */
static const char *stateStr[] =
{
    "CONNECTION_STATE_NULL",
    "CONNECTION_STATE_CONNECTING",
    "CONNECTION_STATE_RINGING",
    "CONNECTION_STATE_CONNECTED",
    "CONNECTION_STATE_DISCONNECTED",
    "CONNECTION_STATE_TIMEOUT"
};

/*
 *  A bool to indicate whether or not the main thread is running
 */
static pj_bool_t thread_quit = 0;

/*
 * The main thread
 */
static pj_thread_t *sipThread;

static int startThread( void *arg );

/*
 *  The local Session Description Protocol body
 */
static Sdp *local_sdp;

/*
 * The instant messaging module
 */
static InstantMessaging *_imModule;

static bool INVITE_AUTO_ANSWER;

static int inv_session_answer();

/*************************************************************************************************/


UserAgent::UserAgent( std::string name, int port )
    : _name(name), _localURI(0)
{
    _state = CONNECTION_STATE_NULL;
    INVITE_AUTO_ANSWER = true;
    _localURI = new URI( port );
    // Instantiate the instant messaging module
    _imModule = new InstantMessaging();
    // To generate a different random number at each time
    srand(time(NULL));
}


UserAgent::~UserAgent(){
    // Delete pointers reference
    delete local_sdp; local_sdp = 0;
    delete _imModule; _imModule = 0;

    pjsip_shutdown();
}


int UserAgent::pjsip_shutdown(){
    // Delete SIP thread
    thread_quit = 1;
    pj_thread_join( sipThread );
    pj_thread_destroy( sipThread );
    sipThread = NULL;

    // Delete SIP endpoint
    if(endpt) {
        pjsip_endpt_destroy( endpt );
        endpt = NULL;
    }
    // Delete memory pools
    if( app_pool ){
        pj_pool_release( app_pool );
        app_pool = NULL;
        pj_caching_pool_destroy(&c_pool);
    }
    pj_shutdown();

    return 0;
}


std::string UserAgent::getConnectionStateStr( connectionState state ){
    std::string res;

    if (state >=0 && state < (connectionState)PJ_ARRAY_SIZE(stateStr))
        res = stateStr[state];
    else
        res = "?UNKNOWN?";
    return res;
}


connectionState UserAgent::getConnectionState( void ){
    return _state;
}

void UserAgent::setInviteAutoAnswer( bool mode ){
    
    INVITE_AUTO_ANSWER = mode;
}

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
    URI *my_uri;
    pjsip_inv_callback inv_cb;


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
    my_uri = getLocalURI();
    listeningPort = (pj_uint16_t) my_uri->_port;
    pj_sockaddr_init( pj_AF_INET(), &local, NULL, listeningPort );
    status = pjsip_udp_transport_start( endpt, &local.ipv4, NULL, 1, NULL );
    if( status != PJ_SUCCESS ){
        // Maybe the port is already in use
        // Try a random one
        listeningPort = RANDOM_SIP_PORT;
        pj_sockaddr_init( pj_AF_INET(), &local, NULL, listeningPort );
        status = pjsip_udp_transport_start( endpt, &local.ipv4, NULL, 1, NULL );
        my_uri->_port = listeningPort;
    }
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Create transaction layer */
    status = pjsip_tsx_layer_init_module( endpt );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Initialize transaction user layer */
    status = pjsip_ua_init_module( endpt, NULL );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    /* Register the callbacks for INVITE session */
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
    local_sdp = new Sdp( my_uri->_hostIP );

    /* Start working threads */
    app_pool = pj_pool_create( &c_pool.factory, "pool", 4000, 4000, NULL );
    pj_thread_create( app_pool, "app", &startThread, NULL, PJ_THREAD_DEFAULT_STACK_SIZE, 0,
                      &sipThread );

    // Update the connection state. The user agent is ready to receive or sent requests
    _state = CONNECTION_STATE_DISCONNECTED;

    PJ_LOG(3, (THIS_FILE, "Ready to accept incoming calls..."));

    return PJ_SUCCESS;
}


int UserAgent::inv_session_create( std::string uri ){
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

    if( _state == CONNECTION_STATE_DISCONNECTED || _state == CONNECTION_STATE_TIMEOUT ) {
        status = pjsip_dlg_create_uac( pjsip_ua_instance(), &from,
                                       NULL,
                                       &to,
                                       NULL,
                                       &dialog );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        // Building the local SDP offer
        local_sdp->createInitialOffer( dialog->pool );

        status = pjsip_inv_create_uac( dialog,
                                       local_sdp->getLocalSDPSession(), 0, &inv_session );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        status = pjsip_inv_invite( inv_session, &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        // Update the connection status. The invite session has been created and sent.
        _state = CONNECTION_STATE_CONNECTING;

        return PJ_SUCCESS;
    }

    else
        return !PJ_SUCCESS;
}


int UserAgent::inv_session_end( void ){
    pj_status_t status;
    pjsip_tx_data *tdata;

    if( _state == CONNECTION_STATE_CONNECTED ){
        status = pjsip_inv_end_session( inv_session, 404, NULL, &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        return PJ_SUCCESS;
    }
    return !PJ_SUCCESS;
}


int UserAgent::inv_session_reinvite( void ){
    pj_status_t status;
    pjsip_tx_data *tdata;

    if( _state == CONNECTION_STATE_CONNECTED ) {
        local_sdp->createInitialOffer( inv_session->dlg->pool );

        status = pjsip_inv_reinvite( inv_session, NULL,
                                     local_sdp->getLocalSDPSession(), &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        return PJ_SUCCESS;
    }
    return !PJ_SUCCESS;
}


int UserAgent::inv_session_accept( void ) {
    
    return inv_session_answer();    
}

static int inv_session_answer(){
    
    pj_status_t status;
    pjsip_tx_data *tdata;

    // Check if the SDP negociation has been succesfully done
    status = local_sdp->startNegociation( app_pool );

    if( status == PJ_SUCCESS ){
        // Create and send a 200(OK) response
        status = pjsip_inv_answer( inv_session, MSG_OK, NULL, NULL, &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        
        _state = CONNECTION_STATE_CONNECTED;
    }
    else{
        // Create and send a 488/Not acceptable here
        // because the SDP negociation failed
        status = pjsip_inv_answer( inv_session, MSG_NOT_ACCEPTABLE_HERE, NULL, NULL, &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    }

    return PJ_SUCCESS;

}

int UserAgent::inv_session_refuse( void ) {
    pj_status_t status;
    pjsip_tx_data *tdata;

    status = pjsip_inv_answer( inv_session, 603, NULL, NULL, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    _state = CONNECTION_STATE_DISCONNECTED;

    return PJ_SUCCESS;
}


int UserAgent::sendInstantMessage( std::string msg ){
    pj_status_t status;

    // Set the current dialog for the instant messaging module
    _imModule->setDialog( inv_session->dlg );
    _imModule->setText( msg );

    if( _state == CONNECTION_STATE_CONNECTED ) {
        // Send the message through the IM module
        status = _imModule->sendMessage();
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        return PJ_SUCCESS;
    }
    return !PJ_SUCCESS;
}


static int startThread( void *arg ){
    PJ_UNUSED_ARG(arg);
    while( !thread_quit )
    {
        pj_time_val timeout = {0, 10};
        pjsip_endpt_handle_events( endpt, &timeout );
    }

    return 0;
}


void UserAgent::setSessionMedia( std::string type, std::string codecs, int port ){
    local_sdp->setSDPMedia( type, codecs, port );
}


std::string UserAgent::mediaToString( void ){
    return local_sdp->mediaToString();
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
    // To avoid unused arguments compilation warnings
    PJ_UNUSED_ARG(e);

    if( inv->state == PJSIP_INV_STATE_DISCONNECTED ){
        cout << "Call state: " << pjsip_get_status_text(inv->cause)->ptr << endl;
        if( strcmp(pjsip_get_status_text(inv->cause)->ptr, "Request Timeout") == 0)
            _state = CONNECTION_STATE_TIMEOUT;
        else
            _state = CONNECTION_STATE_DISCONNECTED;
    }

    else if( inv->state == PJSIP_INV_STATE_CONFIRMED ){
        cout << "Call state confirmed " << endl;
        _state = CONNECTION_STATE_CONNECTED;
    }

    else{
        cout << "Call state changed to " << pjsip_inv_state_name(inv->state) << endl;
        cout << "Call state changed to " << inv->state << endl;
    }
}


static pj_bool_t on_rx_request( pjsip_rx_data *rdata ){
    pj_status_t status;
    pj_str_t reason;
    unsigned options = 0;
    pjsip_dialog* dialog;
    pjsip_tx_data *tdata;
    pjmedia_sdp_session *r_sdp;

    cout << "Callback on_rx_request entered" << endl;

    PJ_UNUSED_ARG( rdata );

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

    // Update the connection state
    _state = CONNECTION_STATE_RINGING;

    if( INVITE_AUTO_ANSWER ){
        inv_session_answer();
    }

    /* Done */

    return PJ_SUCCESS;
}


static pj_bool_t on_rx_response( pjsip_rx_data *rdata ){
    /* Respond statelessly any non-INVITE requests with 500 */
    //if( rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD ) {
    PJ_UNUSED_ARG( rdata );
    cout << "on_rx_response" << endl;
    return PJ_SUCCESS;
}


static void call_on_tsx_state_changed( pjsip_inv_session *inv, pjsip_transaction *tsx,
                                       pjsip_event *e ){
    cout << "transaction state changed to " <<tsx->state << endl;

    PJ_UNUSED_ARG(inv);

    if( tsx->state == PJSIP_TSX_STATE_TERMINATED  &&
        tsx->role == PJSIP_ROLE_UAC ) {
        cout << "UAC: tsx state terminated" << endl;
        //thread_quit = 1;
    }

    else if( tsx->state == PJSIP_TSX_STATE_TRYING &&
             tsx->role == PJSIP_ROLE_UAS ){
        cout << "UAC: tsx state trying" << endl;

        pjsip_rx_data *rdata;
        pjsip_msg *msg;
        pj_status_t status;
        std::string text;

        // Incoming message request
        cout << "Request received" << endl;

        // Retrieve the body message
        rdata = e->body.tsx_state.src.rdata;
        msg = rdata->msg_info.msg;

        // Respond with OK message
        status = pjsip_dlg_respond( inv->dlg, rdata, MSG_OK, NULL, NULL, NULL );

        // Display the message
        text = (char*)msg->body->data;
        _imModule->setResponse( text );
        _imModule->displayResponse();
    }

    else {
        cout << "Transaction state not handled .... " << tsx->state << "for " << tsx->role <<
        endl;
        // TODO Return an error code if transaction failed
        // for instance the peer is not connected
    }
}


static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status ){
    // We need to get the final media choice and send it to gstreamer
    // Maybe we want to start the data streaming now...

    PJ_UNUSED_ARG( inv );
    PJ_UNUSED_ARG( status );

    cout << "on media update" << endl;
    /*
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
     */
}


static void on_rx_offer( pjsip_inv_session *inv, const pjmedia_sdp_session *offer ){
    cout << "Invite session received new offer from peer -" <<  offer->name.ptr << endl;

    PJ_UNUSED_ARG( inv );
    //pjmedia_sdp_session *sdp;
    pj_status_t status;

    local_sdp->receivingInitialOffer(app_pool, (pjmedia_sdp_session*)offer);
    status = pjsip_inv_set_sdp_answer( inv_session, local_sdp->getLocalSDPSession() );
    //PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
}


static void call_on_forked( pjsip_inv_session *inv, pjsip_event *e ){
    PJ_UNUSED_ARG( inv );
    PJ_UNUSED_ARG( e );
    printf(
        "The invite session module has created a new dialog because of forked outgoing request\n");
}


