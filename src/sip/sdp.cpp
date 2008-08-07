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

#include <sstream>
#include <iostream>

static const pj_str_t STR_AUDIO = { (char*)"audio", 5};
static const pj_str_t STR_VIDEO = { (char*)"video", 5};
static const pj_str_t STR_IN = { (char*)"IN", 2 };
static const pj_str_t STR_IP4 = { (char*)"IP4", 3};
static const pj_str_t STR_IP6 = { (char*)"IP6", 3};
static const pj_str_t STR_RTP_AVP = { (char*)"RTP/AVP", 7 };
static const pj_str_t STR_SDP_NAME = { (char*)"call", 7 };
static const pj_str_t STR_SENDRECV = { (char*)"sendrecv", 8 };

Sdp::Sdp( ) : 
    _sdpBody(""), _audioPort(-1), _videoPort(0),  _audiocodecs(0), _videocodecs(0), _local_offer( NULL), negociator(NULL)
{
    setCodecsList();
}

Sdp::Sdp( int aport, int vport ): 
    _sdpBody(""), _audioPort( aport ), _videoPort( vport ), _audiocodecs(0), _videocodecs(0), _local_offer( NULL ), negociator(NULL)
{
    //create_sdp_session( pool );
    setCodecsList();

}

Sdp::~Sdp(){}

void Sdp::addAudioCodec( sdpCodec* codec ){
    this->_audiocodecs.push_back( codec );
}

void Sdp::addVideoCodec( sdpCodec* codec ){}

void Sdp::setCodecsList( void ) {

    sdpCodec *gsm = new sdpCodec("audio", "GSM", 3);
    addAudioCodec( gsm );

    sdpCodec *ulaw = new sdpCodec("audio", "PCMU", 0);
    addAudioCodec( ulaw );

}

void Sdp::displayCodecsList( void ) {

    std::vector<sdpCodec*> audiocodeclist = getAudioCodecsList();
    //std::vector<sdpCodec*>::iterator iter = audiocodeclist.begin();
    //while( iter != audiocodeclist.end() ){
      //  printf( "codec %s %s ", iter->_m_type.c_str(), iter->_name.c_str() );
    //}
    int size = audiocodeclist.size();
    printf( "Display codec list (size = %i)\n" , size);
    int i = 0; 

    while( i < size ) {
        printf("codec %s %s\n", audiocodeclist[i]->getType().c_str(), audiocodeclist[i]->getName().c_str() );
        i++;
    }
}

pjmedia_sdp_media* Sdp::getMediaDescriptorLine( pj_pool_t *pool ) {

    pjmedia_sdp_media* med = PJ_POOL_ZALLOC_T( pool, pjmedia_sdp_media );
    //pjmedia_sdp_media m_ = (pjmedia_sdp_media) malloc( sizeof( pjmedia_sdp_media ) );
    // First the audio media if exists
    if( audioMedia() ){
        /* Standard media info */
        pj_strdup(pool, &med->desc.media, &STR_AUDIO);
        //m->desc.port = pj_sockaddr_get_port(addr0);
        med->desc.port_count = 1;
        pj_strdup (pool, &med->desc.transport, &STR_RTP_AVP);
        /* Init media line and attribute list */
        med->desc.fmt_count = 0;
        med->attr_count = 0;
    }
    // Then the video media
    if( videoMedia() ){
    }

    return med;

}


void Sdp::createLocalOffer( pj_pool_t *pool ){ 

    /* Create and initialize basic SDP session */
    this->_local_offer = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_session);

    /* Initialize the fields of the struct */
    this->_local_offer->origin.user = pj_str( (char*) "propulseART" );
    this->_local_offer->origin.version = 0; 
    this->_local_offer->origin.net_type = STR_IN;
    this->_local_offer->origin.addr_type = STR_IP4;
    this->_local_offer->origin.addr = pj_str( (char*) "192.168.1.230");

    this->_local_offer->name = STR_SDP_NAME;

    /* Connection information */
    this->_local_offer->conn = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_conn);
    this->_local_offer->conn->net_type = _local_offer->origin.net_type;
    this->_local_offer->conn->addr_type = _local_offer->origin.addr_type;
    this->_local_offer->conn->addr = _local_offer->origin.addr;

    /* SDP time and attributes */
    this->_local_offer->time.start = this->_local_offer->time.stop = 0;
    this->_local_offer->attr_count = 0;

    /* Create media stream 0 */
    this->_local_offer->media_count = 1;
    this->_local_offer->media[0] = getMediaDescriptorLine( pool );

    toString();

}

void Sdp::createInitialOffer( pj_pool_t* pool ){

    // Build the SDP session descriptor
    // createLocalOffer()
    //
    // Then create SDP negociator instance
    // pjmedia_sdp_neg_create_w_local_offer()

}

void Sdp::receivingInitialOffer( pj_pool_t* pool, pjmedia_sdp_session* remote ){

    // Create the SDP negociator instance by calling 
    // pjmedia_sdp_neg_create_w_remote_offer with the remote offer, and by providing the local offer ( optional )

}

void Sdp::toString(){
    
    std::ostringstream body;
    using std::cout; using std::endl;
    
    //char* o;//, s, c, t, m;
    pjmedia_sdp_session* sdp = getSDPSession();

    body << "v=" << sdp->origin.version << "\r\n";
    body << "o=" << sdp->origin.user.ptr << " 29972 29972 " << sdp->origin.net_type.ptr << " " << sdp->origin.addr_type.ptr << " " << sdp->origin.addr.ptr << "\r\n"; 
    body << "s=" << sdp->name.ptr << "\r\n";
    body << "c=" << sdp->origin.net_type.ptr << " " << sdp->origin.addr_type.ptr << " " << sdp->origin.addr.ptr << "\r\n";
    body << "t=" << sdp->time.start << " " << sdp->time.stop << "\r\n";
    body << "m=" << sdp->media[0]->desc.media.ptr << " " << getAudioPort() << " " << sdp->media[0]->desc.transport.ptr << " " << getAudioCodecsList()[0]->getPayload() << "\r\n";
    

    cout << body.str() << endl;

}
