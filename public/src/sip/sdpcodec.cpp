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

sdpCodec::sdpCodec( std::string name )
    : _name(name), _m_type(""), _payload(-1), _frequency(-1), _channels(-1)
{
    // A codec is identified by its string name, as described in RFC 3551

    if( name.compare( "PCMU" ) ){
        _m_type = MIME_TYPE_AUDIO;
        _payload = RTP_PAYLOAD_ULAW;
        _frequency = 8000;
        _channels = 1;
    }
    else if ( name.compare( "GSM ") ){
        _m_type = MIME_TYPE_AUDIO;
        _payload = RTP_PAYLOAD_GSM;
        _frequency = 8000;
        _channels = 1;
    }
    else if ( name.compare( "PCMA ") ){
        _m_type = MIME_TYPE_AUDIO;
        _payload = RTP_PAYLOAD_ALAW;
        _frequency = 8000;
        _channels = 1;
    }
}


sdpCodec::sdpCodec( std::string type, std::string name, int payload, int ch )
    : _name(name), _m_type( type ), _payload( payload ), _frequency(-1), _channels(ch) {}

sdpCodec::~sdpCodec(){}

std::string sdpCodec::getPayloadStr() {
    std::ostringstream ret;
    ret << getPayload();
    return ret.str();
}


