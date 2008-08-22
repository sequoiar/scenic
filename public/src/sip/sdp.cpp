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
    :  _sdpMediaList( 0 ), _ip_addr( "" ), _local_offer( NULL), negociator(NULL)
{
    //setCodecsList();
}


Sdp::Sdp( std::string ip_addr )
    : _sdpMediaList(0), _ip_addr( ip_addr ), _local_offer( NULL ), negociator(NULL)
{
    //setCodecsList();
}


Sdp::~Sdp(){}

void Sdp::addMediaToSDP( std::string type, std::string codecs, int port ){
    sdpMedia *media;
    size_t pos;
    std::string tmp;
    int k;
    bool retrieved = false;
    std::vector<sdpMedia*>::iterator iter;

    std::cout << "size= "<< _sdpMediaList.size() << std::endl;

    // If no media of ths type has been already created
    for( k = 0 ; k<(int)_sdpMediaList.size() ; k++ ) {
        if( strcmp( _sdpMediaList[k]->getMediaStr().c_str(), type.c_str()) == 0 ){
            std::cout << "retrieve existing media" << std::endl;
            iter = _sdpMediaList.begin()+k;
            _sdpMediaList.erase(iter);
            //media  = _sdpMediaList[k];
            //retrieved = true;
            break;
        }
    }

    if(!retrieved){
        std::cout << "create new media" << std::endl;
        media = new sdpMedia( type, port );
    }
    // The string codecs can contains multiple codecs,
    // we have to parse by assuming that the delimiter is the '/' char
    while( codecs.find("/", 0) != std::string::npos )
    {
        pos = codecs.find("/", 0);
        tmp = codecs.substr(0, pos);
        codecs.erase(0, pos+1);

        media->addCodec(tmp);
    }

    // Add the media in the list
    if(!retrieved)
        _sdpMediaList.push_back( media );
}


void Sdp::getMediaDescriptorLine( sdpMedia *media, pj_pool_t *pool,
                                  pjmedia_sdp_media** p_med ) {
    pjmedia_sdp_media* med;
    pjmedia_sdp_rtpmap rtpmap;
    pjmedia_sdp_attr *attr;
    sdpCodec *codec;
    int count, i;

    med = PJ_POOL_ZALLOC_T( pool, pjmedia_sdp_media );

    // Get the right media format
    pj_strdup(pool, &med->desc.media,
              ( media->getType() == MIME_TYPE_AUDIO ) ? &STR_AUDIO : &STR_VIDEO );
    med->desc.port_count = 1;
    std::cout <<  media->getPort() << std::endl;
    med->desc.port = media->getPort();
    pj_strdup (pool, &med->desc.transport, &STR_RTP_AVP);

    // Media format ( RTP payload )
    count = media->getMediaCodecList().size();
    med->desc.fmt_count = count;

    // add the payload list
    for(i=0; i<count; i++){
        codec = media->getMediaCodecList()[i];
        pj_strdup2( pool, &med->desc.fmt[i], codec->getPayloadStr().c_str());

        // Add a rtpmap field for dynamic payload
        if( !codec->isPayloadStatic() ) {
            rtpmap.pt = med->desc.fmt[i];
            rtpmap.enc_name = pj_str( (char*) codec->getName().c_str() );
            rtpmap.clock_rate = codec->getClockrate();
            // Add the channel number only if different from 1
            if( codec->getChannels() > 1 )
                rtpmap.param = pj_str( (char*) codec->getChannelsStr().c_str() );
            else
                rtpmap.param.slen = 0;
            pjmedia_sdp_rtpmap_to_attr( pool, &rtpmap, &attr );
            med->attr[ med->attr_count++] = attr;
        }
    }

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

    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    return PJ_SUCCESS;
}


int Sdp::receivingInitialOffer( pj_pool_t* pool, pjmedia_sdp_session* remote ){
    // Create the SDP negociator instance by calling
    // pjmedia_sdp_neg_create_w_remote_offer with the remote offer, and by providing the local offer ( optional )

    pj_status_t status;
    pjmedia_sdp_neg_state state;

    // Create the SDP negociator instance by calling
    // pjmedia_sdp_neg_create_w_remote_offer with the remote offer, and by providing the local offer ( optional )

    // Build the local offer to respond
    createLocalOffer( pool );

    status = pjmedia_sdp_neg_create_w_remote_offer( pool,
                                                    getLocalSDPSession(), remote, &negociator );
    state = pjmedia_sdp_neg_get_state( negociator );
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
    pjmedia_sdp_media* med;
    int nbMedia, i;

    med = PJ_POOL_ZALLOC_T(pool, pjmedia_sdp_media);
    nbMedia = getSDPMediaList().size();
    this->_local_offer->media_count = nbMedia;

    for( i=0; i<nbMedia; i++ ){
        getMediaDescriptorLine( getSDPMediaList()[i], pool, &med );
        this->_local_offer->media[i] = med;
    }
}


void Sdp::addMedia( sdpMedia *media, int port ){}


std::string Sdp::mediaToString( void ){
    int size, i;
    std::ostringstream res;

    size = _sdpMediaList.size();
    for( i = 0; i < size ; i++ ){
        res << _sdpMediaList[i]->toString();
    }

    res << std::endl;
    return res.str();
}


