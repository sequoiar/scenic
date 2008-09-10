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
#include "logWriter.h"

#include <stdlib.h>
#include <iostream>
#include <boost/python.hpp>

using std::cout;
using std::cerr;
using std::endl;

#define RANDOM_SIP_PORT   rand() % 64000 + 1024

#define PY_CALLBACK_MODULE    "pyCallbacks"

#define CORE_NOTIFICATION   1

/**************** STATIC VARIABLES AND FUNCTIONS (callbacks) **************************/

/*
 * Retrieve the SDP of the peer contained in the offer
 *
 * @param rdata	The request data
 * @param r_sdp	The pjmedia_sdp_media to stock the remote SDP
 */
static void getRemoteSDPFromOffer( pjsip_rx_data *rdata, pjmedia_sdp_session** r_sdp );

/*
 * Process an invite request and send the response.
 * 200/OK if SDP negociation succeed, not acceptable response otherwise
 *
 * @return int	PJ_SUCCESS on success
 *      1 otherwise
 */
static int inv_session_answer();

/*
 * Thread entry point
 */
static int startThread( void *arg );

/*
 * Callback function to redirect pjsip log output to our own log system
 *
 * @param level	The pjsip log level. Between 0 (disable), 1(error only) to 6 (maximum)
 * @param data  The log message
 * @param len	The log message length
 */
static void pjsipLogWriter( int level, const char *data, int len );

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
    "CONNECTION_STATE_READY",
    "CONNECTION_STATE_CONNECTING",
    "CONNECTION_STATE_INCOMING",
    "CONNECTION_STATE_CONNECTED",
    "CONNECTION_STATE_DISCONNECTED",
    "CONNECTION_STATE_TIMEOUT",
    "CONNECTION_STATE_NOT_ACCEPTABLE"
};

static errorCode _error;

static const char *errorReason[] =
{
    "NO_ERROR",
    "ERROR_INIT_ALREADY_DONE",
    "ERROR_SHUTDOWN_ALREADY_DONE",
    "ERROR_HOST_UNREACHABLE",
    "ERROR_NO_COMPATIBLE_MEDIA",
    "ERROR_CONNECTION_NOT_READY",
    "ERROR_NOT_CONNECTED"
};

/*
 * A bool to indicate whether or not the main thread is running
 */
static pj_bool_t thread_quit = 0;

/*
 * The main thread
 */
static pj_thread_t *sipThread;

/*
 *  The local Session Description Protocol body
 */
static Sdp *local_sdp;

/*
 * The instant messaging module
 */
static InstantMessaging *_imModule;

/*
 * Should auto answer on a new invite request or wait for the user accept
 */
static answerMode _answerMode;

static PyThreadState* mainThreadState;

static void py_connection_made( void );
static void py_connection_end( void );
static void py_connection_failed( std::string reason );
static void py_connection_incoming( void );
static void py_connection_message( std::string msg );
static void py_connection_media_choice( std::string codec );

/*************************************************************************************************/


UserAgent::UserAgent( std::string name, int port )
    : _name(name), _localURI(0)
{
    // The state of the connection
    _state = CONNECTION_STATE_NULL;

    // The error message
    _error = NO_ERROR;

    // Default behaviour on new incoming invite request
    _answerMode = ANSWER_MODE_AUTO;

    _localURI = new URI( port );

    // Instantiate the instant messaging module
    _imModule = new InstantMessaging();

    // To generate a different random number at each time
    // Useful for the random port selection if the default one is used
    srand(time(NULL));
}


UserAgent::~UserAgent(){
    // Return some memory to the heap
    delete local_sdp; local_sdp = 0;
    delete _imModule; _imModule = 0;

    // Shutdown the pjsip library
    pjsip_shutdown();
}


void UserAgent::initPython(){
    // Initialize python
    Py_Initialize();
    // Initialize thread support
    PyEval_InitThreads();
    // Save a pointer to the main PyThreadState object
    mainThreadState = PyThreadState_Get();
    // Release the lock
    PyEval_ReleaseLock();
}


void UserAgent::python_shutdown(){
    // Shut down the interpreter
    PyInterpreterState * mainInterpreterState = mainThreadState->interp;
    // create a thread state object for this thread
    PyThreadState * myThreadState = PyThreadState_New(mainInterpreterState);
    PyThreadState_Swap(myThreadState);
    PyEval_AcquireLock();
    Py_Finalize();
}


int UserAgent::pjsip_shutdown(){
    if( _state == CONNECTION_STATE_NULL ) {
        _error = ERROR_SHUTDOWN_ALREADY_DONE;
        return 0;
    }
    if( _state == CONNECTION_STATE_CONNECTED )
        inv_session_end();
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
    //python_shutdown();
    _state = CONNECTION_STATE_NULL;
    _error = NO_ERROR;

    return 0;
}


connectionState UserAgent::getConnectionState( void ){
    return _state;
}


errorCode UserAgent::getErrorCode( void ){
    return _error;
}


std::string UserAgent::getConnectionStateStr( connectionState state ){
    std::string res;

    // Look for the connection state equivalent string form
    // Check first if the index is acceptable
    if (state >=0 && state < (connectionState)PJ_ARRAY_SIZE(stateStr)){
        res = stateStr[state];
    }
    else{
        // We dont know this state
        res = "?UNKNOWN?";
    }
    return res;
}


std::string UserAgent::getErrorReason( errorCode code ){
    std::string res;

    // Check first if the index is acceptable
    if (code >=0 && code < (errorCode)PJ_ARRAY_SIZE(errorReason)){
        res = errorReason[code];
    }
    else{
        // We dont know this code
        res = "?UNKNOWN?";
    }
    return res;
}


void UserAgent::setAnswerMode( int mode ){
    // Validate the parameter
    if( mode == 0 || mode == 1 ){
        // 0 for auto mode
        // 1 for manual mode
        _answerMode = (answerMode)mode;
    }
}


void UserAgent::connection_prepare( void ){
    // Reset the state to READY
    if( _state == CONNECTION_STATE_DISCONNECTED ||
        _state == CONNECTION_STATE_NOT_ACCEPTABLE ||
        _state == CONNECTION_STATE_TIMEOUT ){
        _state = CONNECTION_STATE_READY;
    }
}


std::string UserAgent::getAnswerMode( void ){
    std::string res;

    switch( _answerMode )
    {
        case ANSWER_MODE_AUTO:
            res = "auto";
            break;
        case ANSWER_MODE_MANUAL:
            res = "manual";
            break;
        default:
            res = "unknown mode";
            break;
    }

    return res;
}


bool UserAgent::hasIncomingCall( void ){
    return ( getConnectionState() == CONNECTION_STATE_INCOMING );
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

    if( _state != CONNECTION_STATE_NULL ){
        _error = ERROR_INIT_ALREADY_DONE;
        return !PJ_SUCCESS;
    }
    // Redirect the library log output to our log system
    pj_log_set_level( PJ_LOG_LEVEL );
    pj_log_set_log_func( &pjsipLogWriter );

    // Init SIP module
    init_sip_module();

    initPython();

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
    _state = CONNECTION_STATE_READY;
    _error = NO_ERROR;

    PJ_LOG(3, (THIS_FILE, "Ready to accept incoming calls..."));

    //PyRun_SimpleString("print 'iNit done'\n");
    return PJ_SUCCESS;
}


int UserAgent::inv_session_create( std::string uri ){
    pjsip_dialog *dialog;
    pjsip_tx_data *tdata;
    pj_status_t status;
    URI *remote, *local;
    pj_str_t from, to;
    char tmp[90], tmp1[90];

    // Prepare the connection
    connection_prepare();

    // Check if the default address is called
    if( strcmp( uri.c_str(), "default") == 0 ){
        // the remote is actually the same host, binded on the port 5060
        // Use for local tests
        remote = new URI(DEFAULT_SIP_PORT);
    }

    else
        remote = new URI( uri );
    local = getLocalURI();

    pj_ansi_sprintf( tmp, local->getAddress().c_str() );
    from = pj_str(tmp);

    pj_ansi_sprintf( tmp1, remote->getAddress().c_str() );
    to = pj_str(tmp1);

    if( _state == CONNECTION_STATE_READY ) {
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
    else {
        _error = ERROR_CONNECTION_NOT_READY;
        return !PJ_SUCCESS;
    }
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
    else {
        _error = ERROR_NOT_CONNECTED;
        return !PJ_SUCCESS;
    }
}


int UserAgent::inv_session_reinvite( void ){
    pj_status_t status;
    pjsip_tx_data *tdata;

    if( _state == CONNECTION_STATE_CONNECTED ) {
        status = local_sdp->createInitialOffer( inv_session->dlg->pool );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        status = pjsip_inv_reinvite( inv_session, NULL,
                                     local_sdp->getLocalSDPSession(), &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        return PJ_SUCCESS;
    }

    else{
        _error = ERROR_NOT_CONNECTED;
        return !PJ_SUCCESS;
    }
}


int UserAgent::inv_session_accept( void ) {
    return inv_session_answer();
}


static void getLock( PyThreadState** myThreadState ){
    // Get the global lock
    PyEval_AcquireLock();

    // Create a thread state object for this thread
    *myThreadState = PyThreadState_New( mainThreadState->interp);
    PyThreadState_Swap(*myThreadState);
}


static void releaseLock(PyThreadState* myThreadState){
    // Swap my thread state out of the interpreter
    PyThreadState_Swap(NULL);
    PyThreadState_Clear(myThreadState);
    PyThreadState_Delete(myThreadState);
    PyEval_ReleaseLock();
}


static int inv_session_answer(){
    pj_status_t status;
    pjsip_tx_data *tdata;

    // Check if the SDP negociation has been succesfully done
    status = local_sdp->startNegociation( app_pool );

    if( status == PJ_SUCCESS ){
        // Create and send a 200(OK) response
        status = pjsip_inv_answer( inv_session, PJSIP_SC_OK, NULL, NULL, &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        _state = CONNECTION_STATE_CONNECTED;
        _error = NO_ERROR;
    }
    else{
        // Create and send a 488/Not acceptable here
        // because the SDP negociation failed
        status = pjsip_inv_answer( inv_session, PJSIP_SC_NOT_ACCEPTABLE_HERE, NULL, NULL,
                                   &tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        status = pjsip_inv_send_msg( inv_session, tdata );
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

        _state = CONNECTION_STATE_NOT_ACCEPTABLE;
        _error = ERROR_NO_COMPATIBLE_MEDIA;
    }
    return PJ_SUCCESS;
}


int UserAgent::inv_session_refuse( void ) {
    pj_status_t status;
    pjsip_tx_data *tdata;

    status = pjsip_inv_answer( inv_session, PJSIP_SC_DECLINE, NULL, NULL, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    _state = CONNECTION_STATE_DISCONNECTED;
    _error = NO_ERROR;

    return PJ_SUCCESS;
}


int UserAgent::sendInstantMessage( std::string msg ){
    pj_status_t status;

    if( inv_session ){
        // Set the current dialog for the instant messaging module
        _imModule->setDialog( inv_session->dlg );
        _imModule->setText( msg );
    }
    if( _state == CONNECTION_STATE_CONNECTED ) {
        // Send the message through the IM module
        status = _imModule->sendMessage();
        PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
        return PJ_SUCCESS;
    }
    else {
        _error = ERROR_NOT_CONNECTED;
        return !PJ_SUCCESS;
    }
}


void UserAgent::setSessionMedia( std::string type, std::string codecs, int port,
                                 std::string dir ){
    local_sdp->setSDPMedia( type, codecs, port, dir );
}


std::string UserAgent::mediaToString( void ){
    return local_sdp->mediaToString();
}


static int startThread( void *arg ){
    PJ_UNUSED_ARG(arg);
    while( !thread_quit )
    {
        pj_time_val timeout = {0, 10};
        // Poll for sip events
        pjsip_endpt_handle_events( endpt, &timeout );
    }

    return 0;
}


static void getRemoteSDPFromOffer( pjsip_rx_data *rdata, pjmedia_sdp_session** r_sdp ){
    pjmedia_sdp_session *sdp;
    pjsip_msg *msg;
    pjsip_msg_body *body;

    // Get the message
    msg = rdata->msg_info.msg;
    // Get the body message
    body = msg->body;

    // Parse the remote request to get the sdp session
    pjmedia_sdp_parse( rdata->tp_info.pool, (char*)body->data, body->len, &sdp );

    *r_sdp = sdp;
}


static void pjsipLogWriter( int level, const char *data, int len ){
    PJ_UNUSED_ARG(len);

    // Map the pjsip log level to the log level we use
    switch(level)
    {
        case 1:
            LOG_ERROR(data);
            break;
        case 2:
            LOG_CRITICAL(data);
        case 3:
            break;
        case 4:
            LOG_INFO(data);
            break;
        case 5:
        case 6:
            LOG_DEBUG(data);
            break;
    }
}


static void py_connection_made( void ){
    std::ostringstream inst;
    PyThreadState* myThreadState;

    inst << "import " << PY_CALLBACK_MODULE << " as sip\n" << endl;
    try {
        getLock(&myThreadState);
        cout << "C++ wrote: connection made" << endl;
        PyRun_SimpleString(inst.str().c_str());
        PyRun_SimpleString("sip.connection_made_cb()\n");
        releaseLock(myThreadState);
    }
    catch(boost::python::error_already_set const &)
    {
        PyErr_Print();
    }
}


static void py_connection_end( void ){
    std::ostringstream inst;
    PyThreadState* myThreadState;

    inst << "import " << PY_CALLBACK_MODULE << " as sip\n" << endl;
    try {
        getLock(&myThreadState);
        cout << "C++ wrote: connection end" << endl;
        PyRun_SimpleString(inst.str().c_str());
        PyRun_SimpleString("sip.connection_end_cb()\n");
        releaseLock(myThreadState);
    }
    catch(boost::python::error_already_set const &)
    {
        PyErr_Print();
    }
}


static void py_connection_incoming( void ){
    std::ostringstream inst;
    PyThreadState* myThreadState;

    inst << "import " << PY_CALLBACK_MODULE << " as sip\n" << endl;
    try {
        getLock(&myThreadState);
        cout << "C++ wrote: connection incoming" << endl;
        PyRun_SimpleString(inst.str().c_str());
        PyRun_SimpleString("sip.connection_incoming_cb()\n");
        releaseLock(myThreadState);
    }
    catch(boost::python::error_already_set const &)
    {
        PyErr_Print();
    }
}


static void py_connection_message( std::string msg ){
    std::ostringstream inst;
    PyThreadState* myThreadState;

    PJ_UNUSED_ARG(msg);

    inst << "import " << PY_CALLBACK_MODULE << " as sip\n" << endl;
    try {
        getLock(&myThreadState);
        cout << "C++ wrote: message waiting" << endl;
        PyRun_SimpleString(inst.str().c_str());
        PyObject *pyMsg = PyString_FromString(msg.c_str());
        if(pyMsg){
            PyObject *rep = PyObject_Repr(pyMsg);
            if(rep){
                inst << "sip.connection_message_cb(" << PyString_AsString(rep) << ")\n" <<
                endl;
                Py_DECREF(rep);
            }
            Py_DECREF(pyMsg);
        }
        PyRun_SimpleString(inst.str().c_str());
        releaseLock(myThreadState);
    }
    catch(boost::python::error_already_set const &)
    {
        PyErr_Print();
    }
}


static void py_connection_failed( std::string reason ){
    std::ostringstream inst;
    PyThreadState* myThreadState;

    inst << "import " << PY_CALLBACK_MODULE << " as sip\n" << endl;
    try {
        getLock(&myThreadState);
        cout << "C++ wrote: connection failed" << endl;
        PyObject *pyMsg = PyString_FromString(reason.c_str());
        if(pyMsg){
            PyObject *rep = PyObject_Repr(pyMsg);
            if(rep){
                inst << "sip.connection_failed_cb(" << PyString_AsString(rep) << ")\n" <<
                endl;
                Py_DECREF(rep);
            }
            Py_DECREF(pyMsg);
        }
        PyRun_SimpleString(inst.str().c_str());
        releaseLock(myThreadState);
    }
    catch(boost::python::error_already_set const &)
    {
        PyErr_Print();
    }
}


static void py_connection_media_choice( std::string codec ){
    std::ostringstream inst;
    PyThreadState* myThreadState;

    inst << "import " << PY_CALLBACK_MODULE << " as sip\n" << endl;
    try {
        getLock(&myThreadState);
        cout << "C++ wrote: media choice done" << endl;
        PyObject *pyMsg = PyString_FromString(codec.c_str());
        if(pyMsg){
            PyObject *rep = PyObject_Repr(pyMsg);
            if(rep){
                inst << "sip.connection_media_choice_cb(" << PyString_AsString(rep) <<
                ")\n" <<
                endl;
                Py_DECREF(rep);
            }
            Py_DECREF(pyMsg);
        }
        PyRun_SimpleString(inst.str().c_str());
        releaseLock(myThreadState);
    }
    catch(boost::python::error_already_set const &)
    {
        PyErr_Print();
    }
}


/********************** Callbacks Implementation **********************************/

static void call_on_state_changed( pjsip_inv_session *inv, pjsip_event *e ){
    // To avoid unused arguments compilation warnings
    PJ_UNUSED_ARG(e);

    // The call is terminated
    if( inv->state == PJSIP_INV_STATE_DISCONNECTED ){
        switch( inv->cause )
        {
            case PJSIP_SC_REQUEST_TIMEOUT:
                // The host was probably unreachable: bad address, bad port, ...
                _state = CONNECTION_STATE_TIMEOUT;
                _error = ERROR_HOST_UNREACHABLE;
                if( CORE_NOTIFICATION == 1 )
                    py_connection_failed( errorReason[_error] );
                break;
            case PJSIP_SC_NOT_ACCEPTABLE_HERE:
                _state = CONNECTION_STATE_NOT_ACCEPTABLE;
                _error = ERROR_NO_COMPATIBLE_MEDIA;
                if( CORE_NOTIFICATION == 1 )
                    py_connection_failed( errorReason[_error] );
                break;
            default:
                _state = CONNECTION_STATE_DISCONNECTED;
                _error = NO_ERROR;
                if( CORE_NOTIFICATION == 1 )
                    py_connection_end( );
                break;
        }
    }

    else if( inv->state == PJSIP_INV_STATE_CONFIRMED ){
        // The connection is established
        _state = CONNECTION_STATE_CONNECTED;
        _error = NO_ERROR;
        // Notify the core
        if( CORE_NOTIFICATION == 1 )
            py_connection_made();
    }

    else if( inv->state == PJSIP_INV_STATE_INCOMING ){
        // Incoming invite session
        _state = CONNECTION_STATE_INCOMING;
        if( CORE_NOTIFICATION == 1 )
            py_connection_incoming();
    }

    else{
        // Not handled for now
    }
}


static pj_bool_t on_rx_request( pjsip_rx_data *rdata ){
    pj_status_t status;
    pj_str_t reason;
    unsigned options = 0;
    pjsip_dialog* dialog;
    pjsip_tx_data *tdata;
    pjmedia_sdp_session *r_sdp;


    PJ_UNUSED_ARG( rdata );

    // Respond statelessly any non-INVITE requests with 500
    if( rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD ) {
        if( rdata->msg_info.msg->line.req.method.id != PJSIP_ACK_METHOD ) {
            reason = pj_str((char*)" user agent unable to handle this request ");
            pjsip_endpt_respond_stateless( endpt, rdata, PJSIP_SC_METHOD_NOT_ALLOWED, &reason,
                                           NULL,
                                           NULL );
            return PJ_TRUE;
        }
    }
    // Verify that we can handle the request
    status = pjsip_inv_verify_request( rdata, &options, NULL, NULL, endpt, NULL );
    if( status != PJ_SUCCESS ){
        reason = pj_str((char*)" user agent unable to handle this INVITE ");
        pjsip_endpt_respond_stateless( endpt, rdata, PJSIP_SC_METHOD_NOT_ALLOWED, &reason,
                                       NULL,
                                       NULL );
        return PJ_TRUE;
    }
    // Have to do some stuff here with the SDP
    // We retrieve the remote sdp offer in the rdata struct to begin the negociation
    getRemoteSDPFromOffer( rdata, &r_sdp );
    local_sdp->receivingInitialOffer( rdata->tp_info.pool, r_sdp );

    // Create the local dialog (UAS)
    status = pjsip_dlg_create_uas( pjsip_ua_instance(), rdata, NULL, &dialog );
    if( status != PJ_SUCCESS ) {
        pjsip_endpt_respond_stateless( endpt, rdata, PJSIP_SC_INTERNAL_SERVER_ERROR, &reason,
                                       NULL,
                                       NULL );
        return PJ_TRUE;
    }
    // Specify media capability during invite session creation
    status = pjsip_inv_create_uas( dialog, rdata,
                                   local_sdp->getLocalSDPSession(), 0, &inv_session );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Send a 180/Ringing response
    status = pjsip_inv_initial_answer( inv_session, rdata, PJSIP_SC_RINGING, NULL, NULL,
                                       &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );
    status = pjsip_inv_send_msg( inv_session, tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Auto answer to the invite session or not
    if( _answerMode == ANSWER_MODE_AUTO ){
        inv_session_answer();
    }
    /* Done */

    return PJ_SUCCESS;
}


static pj_bool_t on_rx_response( pjsip_rx_data *rdata ){
    /* Respond statelessly any non-INVITE requests with 500 */
    //if( rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD ) {
    PJ_UNUSED_ARG( rdata );
    return PJ_SUCCESS;
}


static void call_on_tsx_state_changed( pjsip_inv_session *inv, pjsip_transaction *tsx,
                                       pjsip_event *e ){
    PJ_UNUSED_ARG(inv);

    if( tsx->state == PJSIP_TSX_STATE_TERMINATED  &&
        tsx->role == PJSIP_ROLE_UAC ) {}

    else if( tsx->state == PJSIP_TSX_STATE_TRYING &&
             tsx->role == PJSIP_ROLE_UAS ){
        // Incoming message request

        pjsip_rx_data *rdata;
        pjsip_msg *msg;
        pj_status_t status;
        std::string text;

        // Retrieve the body message
        rdata = e->body.tsx_state.src.rdata;
        msg = rdata->msg_info.msg;

        // Respond with OK message
        status = pjsip_dlg_respond( inv->dlg, rdata, PJSIP_SC_OK, NULL, NULL, NULL );
        text = (char*)msg->body->data;

        // Notify the core of an incoming message
        if(CORE_NOTIFICATION == 1)
            py_connection_message( text );
        // Display the message
        _imModule->setResponse( text );
        _imModule->displayResponse();
    }

    else {
        // TODO Return an error code if transaction failed
        // for instance the peer is not connected
    }
}


static void call_on_media_update( pjsip_inv_session *inv, pj_status_t status ){
    // We need to get the final media choice and send it to the core
    // Maybe we want to start the data streaming now...

    PJ_UNUSED_ARG( inv );
    PJ_UNUSED_ARG( status );

    int nbMedia;
    int nbCodecs;
    std::string codec;
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
        // Retrieve the media 
        media = r_sdp->media[i];
        // Retrieve the payload
        nbCodecs = media->desc.fmt_count;  // Must be one
        printf("Codec count: %i\n", nbCodecs);
        for( j=0 ; j<nbCodecs ; j++ ){
            codec = media->desc.fmt[j].ptr;
            if( CORE_NOTIFICATION == 1 )
                py_connection_media_choice(codec);
        }
        // TODO Retrieve the rtpmap attribute to get the encoding name

    }
}


static void on_rx_offer( pjsip_inv_session *inv, const pjmedia_sdp_session *offer ){
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


