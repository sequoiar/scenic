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

#include <pjlib.h>
#include <sstream>
#include <iostream>

static const pj_str_t STR_AUDIO = { (char*)"audio", 5};
static const pj_str_t STR_VIDEO = { (char*)"video", 5};
static const pj_str_t STR_IN = { (char*)"IN", 2 };
static const pj_str_t STR_IP4 = { (char*)"IP4", 3};
static const pj_str_t STR_IP6 = { (char*)"IP6", 3};
static const pj_str_t STR_RTP_AVP = { (char*)"RTP/AVP", 7 };
static const pj_str_t STR_SDP_NAME = { (char*)"miville", 7 };
static const pj_str_t STR_SENDRECV = { (char*)"sendrecv", 8 };

Sdp::Sdp( )
    : _audioPort(-1), _videoPort(0), _ip_addr( "" ), _audiocodecs(0), _videocodecs(0),
    _local_offer( NULL), negociator(NULL)
{
    setCodecsList();
}


Sdp::Sdp( std::string ip_addr, int aport, int vport )
    : _audioPort( aport ), _videoPort( vport ), _ip_addr( ip_addr ), _audiocodecs(0),
    _videocodecs(0), _local_offer( NULL ), negociator(NULL)
{
    setCodecsList();
}


Sdp::~Sdp(){}

void Sdp::addAudioCodec( sdpCodec* codec ){
    this->_audiocodecs.push_back( codec );
}


void Sdp::addVideoCodec( sdpCodec* codec ){}

void Sdp::setCodecsList( void ) {
    sdpCodec *gsm = new sdpCodec(MIME_TYPE_AUDIO, "GSM", 3, 1);
    addAudioCodec( gsm );

    sdpCodec *ulaw = new sdpCodec(MIME_TYPE_AUDIO, "PCMU", 0, 1);
    addAudioCodec( ulaw );
}


std::string Sdp::getMediaPayloadList( std::string type ) {
    std::ostringstream list;
    int size;
    int i=0;
    std::vector<sdpCodec*> codecs;

    if( type == MIME_TYPE_AUDIO ) {
        codecs = getAudioCodecsList();
        size = codecs.size();
        for(i=0; i<size; i++){
            list << codecs[i]->getPayload() << " " ;
        }
    }
    list << std::endl;

    return list.str();
}


void Sdp::getMediaDescriptorLine( pj_pool_t *pool, pjmedia_sdp_media** p_med ) {
    pjmedia_sdp_media* med;
    med = PJ_POOL_ZALLOC_T( pool, pjmedia_sdp_media );
    int count;
    //int i;
    // TODO Get the right media format
    if( audioMedia() ){
        // m=audio
        pj_strdup(pool, &med->desc.media, &STR_AUDIO);
        // Audio port
        med->desc.port_count = 1;
        med->desc.port = getAudioPort();
        // RTP/AVP
        pj_strdup (pool, &med->desc.transport, &STR_RTP_AVP);

        // Media format ( RTP payload )
        count = getAudioCodecsList().size();
        med->desc.fmt_count = count;
        // add the payload list
        //for(i=0;i<count;i++){
        // med->desc.fmt[i] = pj_str( (char*) getAudioCodecsList()[i]->getPayloadStr().c_str());
        //}
        med->desc.fmt[0] = pj_str( (char*)"0" );
        med->desc.fmt[1] = pj_str( (char*)"3" );

        // no attributes (rtpmap, fmt , ...)
        med->attr_count = 0;
    }
    // Then the video media
    if( videoMedia() ){}
    *p_med = med;
}


int Sdp::createLocalOffer( pj_pool_t *pool ){
    // Reference: RFC 4566 [5]


    /* Create and initialize basic SDP session */
    this->_local_offer = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_session);
    this->_local_offer->conn = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_conn);

    /* Initialize the fields of the struct */
    sdp_addProtocol();
    sdp_addOrigin();
    sdp_addSessionName();
    sdp_addConnectionInfo();
    sdp_addTiming();
    sdp_addAttributes( pool );
    sdp_addMediaDescription( pool );

    //toString();

    return PJ_SUCCESS;
}


int Sdp::createInitialOffer( pj_pool_t* pool ){
    pj_status_t status;
    pjmedia_sdp_neg_state state;

    // Build the SDP session descriptor
    createLocalOffer( pool );

    // Create the SDP negociator instance with local offer
    status = pjmedia_sdp_neg_create_w_local_offer( pool, getLocalSDPSession(), &negociator);
    state = pjmedia_sdp_neg_get_state( negociator );
    printf( " STATE = %s\n", pjmedia_sdp_neg_state_str( state ) );

    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );


    return PJ_SUCCESS;
}


int Sdp::receivingInitialOffer( pj_pool_t* pool, pjmedia_sdp_session* remote ){
    // Create the SDP negociator instance by calling
    // pjmedia_sdp_neg_create_w_remote_offer with the remote offer, and by providing the local offer ( optional )

    pj_status_t status;
    pjmedia_sdp_neg_state state;

    // Build the local offer to respond
    createLocalOffer( pool );

    status = pjmedia_sdp_neg_create_w_remote_offer( pool,
                                                    getLocalSDPSession(), remote, &negociator );
    state = pjmedia_sdp_neg_get_state( negociator );
    printf( " STATE = %s\n", pjmedia_sdp_neg_state_str( state ) );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    return PJ_SUCCESS;
}


void Sdp::sdp_addProtocol( void ){
    this->_local_offer->origin.version = 0;
}


void Sdp::sdp_addOrigin( void ){
    pj_time_val tv;
    pj_gettimeofday(&tv);

    this->_local_offer->origin.user = pj_str(pj_gethostname()->ptr);
    // Use Network Time Protocol format timestamp to ensure uniqueness.
    this->_local_offer->origin.id = tv.sec + 2208988800UL;
    // The type of network ( IN for INternet )
    this->_local_offer->origin.net_type = STR_IN;
    // The type of address
    this->_local_offer->origin.addr_type = STR_IP4;
    // The address of the machine from which the session was created
    this->_local_offer->origin.addr = pj_str( (char*)_ip_addr.c_str() );
}


void Sdp::sdp_addSessionName( void ){
    this->_local_offer->name = STR_SDP_NAME;
}


void Sdp::sdp_addConnectionInfo( void ){
    this->_local_offer->conn->net_type = _local_offer->origin.net_type;
    this->_local_offer->conn->addr_type = _local_offer->origin.addr_type;
    this->_local_offer->conn->addr = _local_offer->origin.addr;
}


void Sdp::sdp_addTiming( void ){
    this->_local_offer->time.start = this->_local_offer->time.stop = 0;
}


void Sdp::sdp_addAttributes( pj_pool_t *pool ){
    pjmedia_sdp_attr *a;
    this->_local_offer->attr_count = 1;
    a =  PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_attr);
    a->name=STR_SENDRECV;
    _local_offer->attr[0] = a;
}


void Sdp::sdp_addMediaDescription( pj_pool_t* pool ){
    pjmedia_sdp_media* med =  PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_media);
    this->_local_offer->media_count = 1;
    getMediaDescriptorLine( pool, &med );
    this->_local_offer->media[0] = med;
}


void Sdp::toString(){
    std::ostringstream body;
    using std::cout; using std::endl;
    unsigned int i;

    pjmedia_sdp_session* sdp = getLocalSDPSession();

    body << "v=" << sdp->origin.version << "\r\n";
    body << "o=" << sdp->origin.user.ptr << " " << sdp->origin.id << " " << sdp->origin.id <<
    " " << sdp->origin.net_type.ptr << " " << sdp->origin.addr_type.ptr << " " <<
    sdp->origin.addr.ptr << "\r\n";
    body << "s=" << sdp->name.ptr << "\r\n";
    body << "c=" << sdp->origin.net_type.ptr << " " << sdp->origin.addr_type.ptr << " " <<
    sdp->origin.addr.ptr << "\r\n";
    body << "t=" << sdp->time.start << " " << sdp->time.stop << "\r\n";
    body << "a=" << sdp->attr[0]->name.ptr << "\r\n";
    body << "m=" << sdp->media[0]->desc.media.ptr << " " << getAudioPort() << " " <<
    sdp->media[0]->desc.transport.ptr << " ";
    for( i=0; i<sdp->media[0]->desc.fmt_count; i++ ){
        printf(" %i nvlSJvbJSKLBV = %s\n", sdp->media[0]->desc.fmt_count,
               sdp->media[0]->desc.fmt[i].ptr);
        body << sdp->media[0]->desc.fmt[i].ptr << " ";
    }

    cout << body.str() << endl;
}


