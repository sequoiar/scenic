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

#define MIME_TYPE_AUDIO    0
#define MIME_TYPE_VIDEO    1

/*
 * Codecs RTP payload as defined in
 * RFC 3551 - RTP Profile for Audio and Video Conferences with Minimal Control
 */
typedef enum {
    // media type = A - clock rate = 8000 - channels = 1
    RTP_PAYLOAD_ULAW = 0,
    RTP_PAYLOAD_ALAW = 8,
    RTP_PAYLOAD_GSM = 3,
    RTP_PAYLOAD_ILBC = 101,
    RTP_PAYLOAD_SPEEX = 102,

    // media type = V - clock rate = 90000
    RTP_PAYLOAD_H263 = 34,
    RTP_PAYLOAD_H264 = 98
} CodecRTPPayload;

/*
 * Codecs encoding names
 */
#define CODEC_STR_ULAW      "PCMU"
#define CODEC_STR_ALAW      "PCMA"
#define CODEC_STR_GSM       "GSM"
#define CODEC_STR_G726      "G726-32"
#define CODEC_STR_G729A     "G729"
#define CODEC_STR_SPEEX     "speex"
#define CODEC_STR_ILBC      "iLBC"
#define CODEC_STR_JPEG      "JPEG"
#define CODEC_STR_PNG       "PNG"
#define CODEC_STR_H263      "H263"
#define CODEC_STR_H264      "H264"


class sdpCodec
{
    public:
        sdpCodec( int type, std::string name );
        sdpCodec( int type, std::string name, int payload, int ch, int clockrate );
        ~sdpCodec();

        /*
         * Read accessor. Return the type of the media
         * audio / video
         */
        int getType( void ){ return _m_type; }

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
         * Read accessor. Return the codec's clock rate
         */
        int getClockrate( void ){ return _clockrate; }

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
        int _m_type;

        /* The RTP paylaod */
        int _payload;

        /* The codec clock rate */
        int _clockrate;

        /* The number of channels */
        int _channels;
};

#endif // _SDP_CODEC_H
