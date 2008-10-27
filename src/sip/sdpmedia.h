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

#ifndef _SDP_MEDIA
#define _SDP_MEDIA

#include "sdpcodec.h"

#include <vector>

#define DEFAULT_STREAM_DIRECTION    "sendrecv"

/*
 * @file sdpmedia.h
 * @brief   A class to describe a media. It can be either a video codec or an audio codec.
 *          it maintains internally a list of codecs to use in the SDP session and negociation
 */

/*
 * This enum contains the different media stream direction.
 * To be added in the SDP attributes
 * The last one is only here to have to size information, otherwise the enum struct doesn't provide any means to know it
 */
enum streamDirection {
    SEND_RECEIVE,
    SEND_ONLY,
    RECEIVE_ONLY,
    INACTIVE,
    DIR_COUNT
};

/*
 * This enum contains the different media types.
 * To be added in the SDP attributes
 * The last one is only here to have to size information, otherwise the enum struct doesn't provide any means to know it
 */
enum mediaType {
    AUDIO,
    VIDEO,
    APPLICATION,
    TEXT,
    IMAGE,
    MESSAGE,
    MEDIA_COUNT
};

typedef enum streamDirection streamDirection;
typedef enum mediaType mediaType;

class sdpMedia
{
    public:
        sdpMedia( int type );
        sdpMedia( std::string type, int port, std::string dir = DEFAULT_STREAM_DIRECTION);
        ~sdpMedia();

        /*
         * Read accessor. Return the list of codecs
         */
        std::vector<sdpCodec*> get_media_codec_list() { return _codec_list; }

        /*
         * Read accessor. Return the type of media
         */
        mediaType get_media_type() { return _media_type; }

        /*
         * Read accessor. Return the type of media
         */
        std::string get_media_type_str();

        /*
         * Set the media type
         */
        void set_media_type( int type ) { _media_type = (mediaType)type; }

        /*
         * Read accessor. Return the transport port
         */
        int get_port() { return _port; }

        /*
         * Write accessor. Set the transport port
         */
        void set_port( int port ) { _port = port; }

        /*
         * Add a codec in the current media codecs vector
         *
         * @param codec     A pointer on the codec to add
         */
        void add_codec( sdpCodec *codec );

        /*
         * Add a codec in the current media codecs vector
         *
         * @param codec_name     The codec encoding name
         */
        void add_codec( std::string codec_name );

        /*
         * Add a codec in the current media codecs vector
         *
         * @param payload     The payload type
         */
        void add_codec( int payload );

        /*
         * Remove a codec from the current media codecs vector
         *
         * @param codec_name    The codec encoding name
         */
        void remove_codec( std::string codec_name );

        /*
         * Remove all the codecs from the list
         */
        void clear_codec_list( void );

        /*
         * Return a string description of the current media
         */ 
        std::string to_string( void );

        /*
         * Set the stream direction of the current media
         * ie: sendrecv, sendonly,...
         */
        void set_stream_direction( int direction ) { _stream_type = (streamDirection)direction; }

        /*
         * Get the stream direction of the current media
         * ie: sendrecv, sendonly,...
         */
        streamDirection get_stream_direction( void ) { return _stream_type; }

        /*
         * Get the stream direction string description of the current media
         * ie: sendrecv, sendonly,...
         */
        std::string get_stream_direction_str( void );

    private:
        /* The type of media */
        mediaType _media_type;

        /* The media codec vector */
        std::vector< sdpCodec* > _codec_list;

        /* the transport port */
        int _port;

        /* The stream direction */
        streamDirection _stream_type;
};

#endif // _SDP_MEDIA
