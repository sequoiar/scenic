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

#ifndef _SDP_CODEC_H
#define _SDP_CODEC_H

#include <string>
#include <sstream>
#include <map>

#define MIME_TYPE_AUDIO     "audio"
#define MIME_TYPE_VIDEO     "video"

/*
 * Codecs RTP payload as defined in
 * RFC 3551 - RTP Profile for Audio and Video Conferences with Minimal Control
 */
typedef enum {
    // media type = A - clock rate = 8000 - channels = 1
    RTP_PAYLOAD_ULAW = 0,
    RTP_PAYLOAD_ALAW = 8,
    RTP_PAYLOAD_GSM = 3,

    // media type = V - clock rate = 90000
    RTP_PAYLOAD_H263 = 34
} CodecRTPPayload;

//typedef std::map< CodecRTPPayload , std::string > codecRTPMap;

class sdpCodec
{
    public:
        sdpCodec( std::string name );
        sdpCodec( std::string type, std::string name, int payload, int ch );
        ~sdpCodec();

        /*
         * Read accessor. Return the type of the media
         * audio / video
         */
        std::string getType( void ){ return _m_type; }

        /*
         * Read accessor. Return the name of the codec
         */
        std::string getName( void ){ return _name; }

        /*
         * Read accessor. Return the RTP payload
         */
        int getPayload( void ){ return _payload; }

        /*
         * Return the RTP payload under a string form
         */
        std::string getPayloadStr( void );

        /*
         * Tells whether or not the RTp payload is static.
         *
         * @return bool     True if the payload is inferior to 96
         *                  False otherwise
         */
        bool isPayloadStatic( void ) { return _payload < 96; }

    private:
        /* The encoding name of the codec */
        std::string _name;

        /* The media type */
        std::string _m_type;

        /* The RTP paylaod */
        int _payload;

        /* The codec clock rate */
        int _frequency;

        /* The number of channels */
        int _channels;
};

#endif // _SDP_CODEC_H
