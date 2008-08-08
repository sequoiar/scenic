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

#include "sdpcodec.h"


    sdpCodec::sdpCodec( int type, std::string name )
: _name(name), _m_type(-1), _payload(-1), _clockrate(8000), _channels(1)
{
    // A codec is identified by its string name, as described in RFC 3551

    switch( type ) {
        case MIME_TYPE_AUDIO:
            _m_type = MIME_TYPE_AUDIO;
            if( name.compare( CODEC_STR_ULAW ) ){
                _payload = RTP_PAYLOAD_ULAW;
            }
            else if ( name.compare( CODEC_STR_GSM ) ){
                _payload = RTP_PAYLOAD_GSM;
            }
            else if ( name.compare( CODEC_STR_ALAW) ){
                _payload = RTP_PAYLOAD_ALAW;
            }
            else if ( name.compare( CODEC_STR_ILBC) ){
                _payload = RTP_PAYLOAD_ILBC;
            }
            else if ( name.compare( CODEC_STR_SPEEX) ){
                _payload = RTP_PAYLOAD_SPEEX;
            }
            else{
                // unsupported audio codec
            }
            break;

        case MIME_TYPE_VIDEO:
            _m_type = MIME_TYPE_VIDEO;
            if( name.compare( CODEC_STR_H263 ) ){
                _payload = RTP_PAYLOAD_H263;
            }
            else if( name.compare( CODEC_STR_H264 ) ){
                _payload = RTP_PAYLOAD_H264;
            }
            else{
                // unsupported video codec
            }
            break;

        default:
            // unsupported media type
            break;
    }
}

sdpCodec::sdpCodec( int type, std::string name, int payload, int ch, int clockrate )
    : _name(name), _m_type( type ), _payload( payload ), _clockrate(clockrate),
    _channels(ch) {}

    sdpCodec::~sdpCodec(){}

    std::string sdpCodec::getPayloadStr() {
        std::ostringstream ret;
        ret << getPayload();
        return ret.str();
    }


