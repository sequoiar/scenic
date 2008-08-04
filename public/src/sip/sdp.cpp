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

#include "sdp.h"

static const pj_str_t STR_AUDIO = { (char*)"audio", 5};
static const pj_str_t STR_VIDEO = { (char*)"video", 5};
static const pj_str_t STR_IN = { (char*)"IN", 2 };
static const pj_str_t STR_IP4 = { (char*)"IP4", 3};
static const pj_str_t STR_IP6 = { (char*)"IP6", 3};
static const pj_str_t STR_RTP_AVP = { (char*)"RTP/AVP", 7 };
static const pj_str_t STR_SDP_NAME = { (char*)"call", 7 };
static const pj_str_t STR_SENDRECV = { (char*)"sendrecv", 8 };

Sdp::Sdp( ) : 
    _sdpBody(""), _audioPort(-1), _videoPort(0),  _audiocodecs(0), _videocodecs(0), _sdpSession( NULL)
{

}

Sdp::Sdp( int aport, int vport ): 
    _sdpBody(""), _audioPort( aport ), _videoPort( vport ), _audiocodecs(0), _videocodecs(0), _sdpSession( NULL )
{
    //create_sdp_session( pool );

}

Sdp::~Sdp(){}

void Sdp::addAudioCodec( sdpCodec* codec ){}

void Sdp::addVideoCodec( sdpCodec* codec ){}

void Sdp::setSDPSession( pj_pool_t *pool ){ 

    const pj_sockaddr *addr0;
    pjmedia_sdp_media *m;

    /* Create and initialize basic SDP session */
    this->_sdpSession = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_session);

    /* Initialize the fields of the struct */
    this->_sdpSession->origin.user = pj_str( (char*) "propulseART" );
    this->_sdpSession->origin.net_type = STR_IN;
    //TODO Check the address family ( IPv6 )
    this->_sdpSession->origin.addr_type = STR_IP4;
    //TODO Get dynamically the IP address
    this->_sdpSession->origin.addr = pj_str((char*) "192.168.1.230");

    if (addr0->addr.sa_family == pj_AF_INET()) {
        this->_sdpSession->origin.addr_type = STR_IP4;
        pj_strdup2(pool, &this->_sdpSession->origin.addr,
                pj_inet_ntoa(addr0->ipv4.sin_addr));
    } else if (addr0->addr.sa_family == pj_AF_INET6()) {
        char tmp_addr[PJ_INET6_ADDRSTRLEN];

        this->_sdpSession->origin.addr_type = STR_IP6;
        pj_strdup2(pool, &this->_sdpSession->origin.addr,
                pj_sockaddr_print(addr0, tmp_addr, sizeof(tmp_addr), 0));

    } else {
        pj_assert(!"Invalid address family");
        //return PJ_EAFNOTSUP;
    }


    this->_sdpSession->name = STR_SDP_NAME;

    /* Connection information */
    this->_sdpSession->conn = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_conn);
    this->_sdpSession->conn->net_type = _sdpSession->origin.net_type;
    this->_sdpSession->conn->addr_type = _sdpSession->origin.addr_type;
    this->_sdpSession->conn->addr = _sdpSession->origin.addr;

    /* SDP time and attributes. */
    this->_sdpSession->time.start = this->_sdpSession->time.stop = 0;
    this->_sdpSession->attr_count = 0;

    /* Create media stream 0: */
    this->_sdpSession->media_count = 1;
    m = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_media);
    this->_sdpSession->media[0] = m;

    /* Standard media info: */
    pj_strdup(pool, &m->desc.media, &STR_AUDIO);
    m->desc.port = pj_sockaddr_get_port(addr0);
    m->desc.port_count = 1;
    pj_strdup (pool, &m->desc.transport, &STR_RTP_AVP);

    /* Init media line and attribute list. */
    m->desc.fmt_count = 0;
    m->attr_count = 0;

}
