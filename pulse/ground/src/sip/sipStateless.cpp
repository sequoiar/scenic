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

#include <iostream>
#include <cassert>

// includes to get local ip address

#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <pjsip.h>
#include <pjlib-util.h>
#include <pjlib.h>

#include "sip.h"
#include "sipPrivate.h"

/* If this macro is set, UDP transport will be initialized at port 5060 */
#define HAS_UDP_TRANSPORT

/* If this macro is set, TCP transport will be initialized at port 5060 */
#define HAS_TCP_TRANSPORT   (1 && PJ_HAS_TCP)

/* Global SIP endpoint */
static pjsip_endpoint *sip_endpt;

static pj_caching_pool cp;
static pj_pool_t *pool;
/* What response code to be sent (default is 501/Not Implemented) */
static int code = PJSIP_SC_NOT_IMPLEMENTED;

/* Additional header list */
struct pjsip_hdr hdr_list;

/* Callback to handle response to our request */
static pj_bool_t on_rx_response(pjsip_rx_data * rdata)
{
    SipSingleton *sip = SipSingleton::Instance();

//    if ((rdata->msg_info.msg->line.req.method.id == PJSIP_ACK_METHOD)
//        &&   
    if ((rdata->msg_info.msg->body != NULL) && (rdata->msg_info.msg->line.status.code == 200))
    {
#if 0
        PJ_LOG(3, (__FILE__, "response body:%s", rdata->msg_info.msg->body->data));
#endif
        sip->rx_res((const char *) rdata->msg_info.msg->body->data, rdata->msg_info.msg->body->len);
    }
    else
        return PJ_FALSE;

    return PJ_TRUE;
}

/* Callback to handle incoming requests. */
static pj_bool_t on_rx_request(pjsip_rx_data * rdata)
{
    /* Respond (statelessly) all incoming requests (except ACK!)
     * with 501 (Not Implemented)
     */
    SipSingleton *sip = SipSingleton::Instance();

    if (rdata->msg_info.msg->line.req.method.id != PJSIP_ACK_METHOD)
    {
        if (rdata->msg_info.msg->line.req.method.id == PJSIP_INVITE_METHOD)
        {
            pjsip_msg_body *body;

            pj_str_t t = pj_str((char *) "text");
            pj_str_t s = pj_str((char *) "plain");
            pj_str_t data = pj_str((char *) sip->rx_invite((const char *)
                                                           rdata->msg_info.msg->body->data,
                                                           rdata->msg_info.msg->body->len));

            body = pjsip_msg_body_create(pool, &t, &s, &data);

#if 0
            PJ_LOG(3, (__FILE__, "request body:%s", rdata->msg_info.msg->body->data));
#endif

            pjsip_endpt_respond_stateless(sip_endpt, rdata, 200, NULL, &hdr_list, body);
        }
        else
            pjsip_endpt_respond_stateless(sip_endpt, rdata, code, NULL, &hdr_list, NULL);
    }
    return PJ_TRUE;
}

const pjsip_method message_method = {
    PJSIP_OTHER_METHOD,
    {(char *) "MESSAGE", 7}
};

char from_port[8];
char to_port[8];
char from_addr[64];
char to_addr[64];

void send_request(const char *str)
{
    static char target[64];
    static char from[64];
    static char to[64];

    sprintf(target, "sip:someuser@%s:%s", to_addr, to_port);
    sprintf(from, "\"Local User\" <sip:localuser@%s:%s>", from_addr, from_port);
    sprintf(to, "\"Remote User\" <sip:remoteuser@%s:%s>", to_addr, to_port);

    std::cout << "Sending request from " << from << std::endl;
    std::cout << "To " << to << std::endl;

    {
        pj_str_t str_target = pj_str(target);
        pj_str_t str_from = pj_str(from);
        pj_str_t str_to = pj_str(to);
        pj_str_t str_contact = str_from;
        pj_status_t status;
        pjsip_tx_data *request;
        pj_str_t body = pj_str((char *) str);
        pjsip_method method;

        pjsip_method_set(&method, PJSIP_INVITE_METHOD);
        status = pjsip_endpt_create_request(sip_endpt, &method,
                                            &str_target, &str_from, &str_to,
                                            &str_contact, NULL, -1, &body, &request);
        assert(status == PJ_SUCCESS);

        status = pjsip_endpt_send_request_stateless(sip_endpt, request, NULL, NULL);
        assert(status == PJ_SUCCESS);
    }
}

/*
 * main()
 *
 */
int sip_init()
{
    static pjsip_module mod_app = {
        NULL, NULL,             /* prev, next.              */
        {(char *) "mod-app", 7},        /* Name.                    */
        -1,                     /* Id               */
        PJSIP_MOD_PRIORITY_APPLICATION, /* Priority             */
        NULL,                   /* load()                   */
        NULL,                   /* start()                  */
        NULL,                   /* stop()                   */
        NULL,                   /* unload()                 */
        &on_rx_request,         /* on_rx_request()          */
        &on_rx_response,        /* on_rx_response()         */
        NULL,                   /* on_tx_request.           */
        NULL,                   /* on_tx_response()         */
        NULL,                   /* on_tsx_state()           */
    };
    pj_status_t status;

    // set log level
    pj_log_set_level(0);

    /* Must init PJLIB first: */
    status = pj_init();
    assert(status == PJ_SUCCESS);

    /* Then init PJLIB-UTIL: */
    status = pjlib_util_init();
    assert(status == PJ_SUCCESS);

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
        assert(status == PJ_SUCCESS);
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

        status = pjsip_udp_transport_start(sip_endpt, &addr, NULL, 1, NULL);
        assert(status == PJ_SUCCESS);
    }
#endif

#if 0 && HAS_TCP_TRANSPORT
    /*
     * Add TCP transport, with hard-coded port
     */
    {
        pj_sockaddr_in addr;

        addr.sin_family = pj_AF_INET();
        addr.sin_addr.s_addr = 0;
        addr.sin_port = pj_htons(atoi(from_port));

        status = pjsip_tcp_transport_start(sip_endpt, &addr, 1, NULL);
        assert(status == PJ_SUCCESS);
    }
#endif

    /*
     * Register our module to receive incoming requests.
     */
    status = pjsip_endpt_register_module(sip_endpt, &mod_app);
    assert(status == PJ_SUCCESS);

    /* Done. Loop forever to handle incoming events. */

    PJ_LOG(3, (__FILE__, "Press Ctrl-C to quit.."));

    return 0;
}

unsigned int sip_handle_events(void)
{
    unsigned int count;
    pj_time_val maxTimeOut;
    maxTimeOut.sec = 0;
    maxTimeOut.msec = 0;

    // has a queue of events, returns on event or after 10ms
    pjsip_endpt_handle_events2(sip_endpt, &maxTimeOut, &count);
    return count;
}

void sip_set_local(const char *port)
{
    strcpy(from_port, port);
    sip_default_local_host();   // gets IP from this machine
}

void sip_set_local(const char *host, const char *port)
{
    strcpy(from_addr, host);
    strcpy(from_port, port);
}

void sip_set_remote(const char *host, const char *port)
{
    strcpy(to_addr, host);
    strcpy(to_port, port);
}

bool isInetAddress(const char *ip)
{
    const char prefix[] = "127";
    const short prefixLength = strlen(prefix);

    if (strncmp(ip, prefix, prefixLength) != 0)
        return true;
    else
        return false;
}

void sip_default_local_host()
{
    int i;
    int s = socket(PF_INET, SOCK_STREAM, 0);

    for (i = 1;; i++)
    {
        struct ifreq ifr;
        struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;
        char *ip;

        ifr.ifr_ifindex = i;
        if (ioctl(s, SIOCGIFNAME, &ifr) < 0)
        {
            if (isInetAddress(ip))
            {
                strncpy(from_addr, ip, strlen(ip));
                std::cout << "Local host address is: " << from_addr << std::endl;
                break;
            }
        }

        /* now ifr.ifr_name is set */
        if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
            continue;

        ip = inet_ntoa(sin->sin_addr);
    }

    close(s);
}

#if 0
int sip_pass_args(int argc, char *argv[])
{

    switch (argc)
    {
    case 5:                    // 4 args
        strcpy(from_addr, argv[1]);
        strcpy(to_port, argv[4]);
        strcpy(to_addr, argv[3]);
        strcpy(from_port, argv[2]);
        break;

    case 2:                    // 1 or more args (but not 4)
        to_port[0] = 0;
        strcpy(from_port, argv[1]);
        break;

    default:                   // no args
        std::cerr << "Usage: " << std::endl
            << "sipStateless <fromIP> <fromPort> <toIP> <toPort>"
            << std::endl << "or" << std::endl << "sipStateless <listenPort>" << std::endl;
        return -1;
    }

    return 0;
}
#endif
