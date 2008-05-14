/* $Id: sipstateless.c 1937 2008-04-22 18:32:53Z bennylp $ */
/* 
 * Copyright (C) 2003-2007 Benny Prijono <benny@prijono.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

/**
 * sipcore.c
 *
 * A simple program to respond any incoming requests (except ACK, of course!)
 * with any status code (taken from command line argument, with the default
 * is 501/Not Implemented).
 */


/* Include all headers. */
#include <pjsip.h>
#include <pjlib-util.h>
#include <pjlib.h>


/* If this macro is set, UDP transport will be initialized at port 5060 */
#define HAS_UDP_TRANSPORT

/* If this macro is set, TCP transport will be initialized at port 5060 */
#define HAS_TCP_TRANSPORT   (1 && PJ_HAS_TCP)

/* Log identification */
#define THIS_FILE	"sipstateless.c"


/* Global SIP endpoint */
static pjsip_endpoint *sip_endpt;

static pj_caching_pool cp;
static pj_pool_t *pool;
/* What response code to be sent (default is 501/Not Implemented) */
static int code = PJSIP_SC_NOT_IMPLEMENTED;

/* Additional header list */
struct pjsip_hdr hdr_list;


static pj_bool_t on_rx_response( pjsip_rx_data *rdata )
{

    //    if (rdata->msg_info.msg->line.req.method.id == PJSIP_OTHER_METHOD)
    //{
    if(rdata->msg_info.msg->body != NULL)
        PJ_LOG(3,(THIS_FILE, "response body:%s", 
               rdata->msg_info.msg->body->data));
    //}
    return PJ_TRUE;
}


/* Callback to handle incoming requests. */
static pj_bool_t on_rx_request( pjsip_rx_data *rdata )
{
    /* Respond (statelessly) all incoming requests (except ACK!) 
     * with 501 (Not Implemented)
     */
    if (rdata->msg_info.msg->line.req.method.id != PJSIP_ACK_METHOD) 
    {
        if (rdata->msg_info.msg->line.req.method.id == PJSIP_OTHER_METHOD)
        {
            pjsip_msg_body *body;
            pj_str_t t = pj_str("text");
            pj_str_t s = pj_str("plain");
            pj_str_t data = pj_str("Got Something");
            body = pjsip_msg_body_create(pool,&t,&s,&data);

            PJ_LOG(3,(THIS_FILE, "request body:%s",
                   rdata->msg_info.msg->body->data));

            pjsip_endpt_respond_stateless(sip_endpt, rdata, 200, NULL, 
                                          &hdr_list, body);
        }
        else
            pjsip_endpt_respond_stateless(sip_endpt, rdata, code, NULL, 
                                          &hdr_list, NULL);
    }
    return PJ_TRUE;
}

const pjsip_method message_method = 
{
    PJSIP_OTHER_METHOD,
    { "MESSAGE", 7 }
};


char from_port[8];
char to_port[8];
char from_addr[64];
char to_addr[64];

void send_request(pjsip_endpoint *endp)
{
    static char target[64];
    static char from[64];
    static char to[64];

    sprintf(target, "sip:someuser@%s:%s", to_addr, to_port);
    sprintf(from, "\"Local User\" <sip:localuser@%s:%s>", from_addr, from_port);
    sprintf(to, "\"Remote User\" <sip:remoteuser@%s:%s>", to_addr,to_port);

    {
        pj_str_t str_target = pj_str(target);
        pj_str_t str_from = pj_str(from);
        pj_str_t str_to = pj_str(to);
        pj_str_t str_contact = str_from;
        pj_status_t status;
        pjsip_tx_data *request;
        pj_str_t body = pj_str("XHello World.");

        status = pjsip_endpt_create_request(endp, &message_method, &str_target,
                                            &str_from, &str_to,&str_contact, 
                                            NULL, -1, &body,&request);

        if (status != PJ_SUCCESS) 
        {
            exit(-1); 
            //app_perror("    error: unable to create request", status);
            //    return status;
        }
        status = pjsip_endpt_send_request_stateless(endp, request, NULL, NULL);	
    }
}




/*
 * main()
 *
 */
int sip_init(const char *from_addr,const char *from_port,const char *to_addr,const char *to_port)
{
    pjsip_module mod_app = 
    {
        NULL, NULL,		    /* prev, next.		*/
        {(char*) "mod-app", 7 },	    /* Name.			*/
        -1,				    /* Id		*/
        PJSIP_MOD_PRIORITY_APPLICATION, /* Priority		*/
        NULL,			    /* load()			*/
        NULL,			    /* start()			*/
        NULL,			    /* stop()			*/
        NULL,			    /* unload()			*/
        &on_rx_request,		    /* on_rx_request()		*/
        &on_rx_response,			    /* on_rx_response()		*/
        NULL,			    /* on_tx_request.		*/
        NULL,			    /* on_tx_response()		*/
        NULL,			    /* on_tsx_state()		*/
    };
    pj_status_t status;

    /* Must init PJLIB first: */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Then init PJLIB-UTIL: */
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    /* Create global endpoint: */
    {
        /* Endpoint MUST be assigned a globally unique name.
         * Ideally we should put hostname or public IP address, but
         * we'll just use an arbitrary name here.
         */

        /* Create the endpoint: */
        status = pjsip_endpt_create(&cp.factory, "sipstateless", &sip_endpt);
        PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    }

    pj_optind = 0;
    pj_list_init(&hdr_list);

    if (pool == NULL) 
        pool = pj_pool_create(&cp.factory, "sipstateless", 1000, 1000, NULL);
    /* 
     * Add UDP transport, with hard-coded port 
     */

#ifdef HAS_UDP_TRANSPORT
    {
        pj_sockaddr_in addr;

        addr.sin_family = pj_AF_INET();
        addr.sin_addr.s_addr = 0;
        addr.sin_port = pj_htons(atoi(from_port));

        status = pjsip_udp_transport_start( sip_endpt, &addr, NULL, 1, NULL);

        if (status != PJ_SUCCESS) 
        {
            PJ_LOG(3,(THIS_FILE,
                   "Error starting UDP transport (port in use?)"));
            return 1;
        }
    }
#endif

#if HAS_TCP_TRANSPORT
    /* 
     * Add UDP transport, with hard-coded port 
     */
    {
        pj_sockaddr_in addr;

        addr.sin_family = pj_AF_INET();
        addr.sin_addr.s_addr = 0;
        addr.sin_port = pj_htons(atoi(from_port));

        status = pjsip_tcp_transport_start(sip_endpt, &addr, 1, NULL);
        if (status != PJ_SUCCESS) 
        {
            PJ_LOG(3,(THIS_FILE, 
                   "Error starting TCP transport (port in use?)"));
            return 1;
        }
    }
#endif

    /*
     * Register our module to receive incoming requests.
     */
    status = pjsip_endpt_register_module(sip_endpt, &mod_app);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    if(to_port[0])
    {
        send_request(sip_endpt);
    }

    /* Done. Loop forever to handle incoming events. */

    PJ_LOG(3, (THIS_FILE, "Press Ctrl-C to quit.."));
 
    for (;;) 
    {
        pjsip_endpt_handle_events(sip_endpt, NULL);
    }
}

void sip_handle_events(void)
{
   pjsip_endpt_handle_events(sip_endpt, NULL);
}

int main(int argc, char *argv[])
{
    switch (argc)
    {
        case 5:         // 4 args
            strcpy(from_addr,argv[1]);
            strcpy(to_port,argv[4]);
            strcpy(to_addr,argv[3]);
            strcpy(from_port,argv[2]);
            break;

        case 1:         // no args
            return -1;

        default:        // 1 or more args (but not 4)
            to_port[0] = 0;
            strcpy(from_port,argv[1]);
            break;
    }


    sip_init(from_addr,from_port,to_addr,to_port);
    for(;;)
        sip_handle_events();

    return 0;
}


