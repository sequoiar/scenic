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
#include <iostream>

using std::cout;
sdpCodec::sdpCodec( int payload )
    : _name(""), _m_type(-1), _payload(-1), _clockrate(8000), _channels(1)
{
    _payload = payload;
    switch(_payload)
    {
        case RTP_PAYLOAD_ULAW:
            _name = CODEC_STR_ULAW;
            _m_type = MIME_TYPE_AUDIO;
            _clockrate = 8000;
            _channels = 1;
            break;
        case RTP_PAYLOAD_ALAW:
            _name = CODEC_STR_ALAW;
            _m_type = MIME_TYPE_AUDIO;
            _clockrate = 8000;
            _channels = 1;
            break;
        case RTP_PAYLOAD_GSM:
            _name = CODEC_STR_GSM;
            _m_type = MIME_TYPE_AUDIO;
            _clockrate = 8000;
            _channels = 1;
            break;
        case RTP_PAYLOAD_ILBC:
            _name = CODEC_STR_ILBC;
            _m_type = MIME_TYPE_AUDIO;
            _clockrate = 8000;
            _channels = 1;
            break;
        case RTP_PAYLOAD_SPEEX:
            _name = CODEC_STR_SPEEX;
            _m_type = MIME_TYPE_AUDIO;
            _clockrate = 8000;
            _channels = 1;
            break;
        case RTP_PAYLOAD_VORBIS:
            _name = CODEC_STR_VORBIS;
            _m_type = MIME_TYPE_AUDIO;
            _clockrate = 48000;
            _channels = 8;
            break;
        case RTP_PAYLOAD_H263:
            _name = CODEC_STR_H263;
            _m_type = MIME_TYPE_VIDEO;
            _clockrate = 90000;
            _channels = 1;
            break;
        case RTP_PAYLOAD_H264:
            _name = CODEC_STR_H264;
            _m_type = MIME_TYPE_VIDEO;
            _clockrate = 90000;
            _channels = 1;
            break;
    }

}


sdpCodec::sdpCodec( int type, std::string name )
    : _name(name), _m_type(-1), _payload(-1), _clockrate(8000), _channels(1)
{
    // A codec is identified by its string name, as described in RFC 3551

    switch( type )
    {
        case MIME_TYPE_AUDIO:
            _m_type = MIME_TYPE_AUDIO;
            if( name == CODEC_STR_ULAW ) {
                _payload = RTP_PAYLOAD_ULAW;
            }
            else if ( name == CODEC_STR_GSM ) {
                _payload = RTP_PAYLOAD_GSM;
            }
            else if ( name == CODEC_STR_ALAW) {
                _payload = RTP_PAYLOAD_ALAW;
            }
            else if ( name == CODEC_STR_ILBC) {
                _payload = RTP_PAYLOAD_ILBC;
            }
            else if ( name == CODEC_STR_SPEEX) {
                _payload = RTP_PAYLOAD_SPEEX;
            }
            else if ( name == CODEC_STR_VORBIS) {
                _payload = RTP_PAYLOAD_VORBIS;
                _clockrate = 48000;
                _channels = 8;
            }
            else{
                // unsupported audio codec
            }
            break;

        case MIME_TYPE_VIDEO:
            _m_type = MIME_TYPE_VIDEO;
            if( name == CODEC_STR_H263 ){
                _payload = RTP_PAYLOAD_H263;
                _clockrate = 90000;
            }
            else if( name == CODEC_STR_H264 ) {
                _payload = RTP_PAYLOAD_H264;
                _clockrate = 90000;
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

std::string sdpCodec::get_payload_str() {
    std::ostringstream ret;
    ret << get_payload();
    return ret.str();
}


std::string sdpCodec::get_channels_str() {
    std::ostringstream ret;
    ret << get_channels();
    return ret.str();
}


