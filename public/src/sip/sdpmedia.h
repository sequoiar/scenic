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

/*
 * @file sdpmedia.h
 * @brief   A class to describe a media. It can be either a video codec or an audio codec.
 *          it maintains internally a list of codecs to use in the SDP session and negociation
 */

enum streamDirection {
    SEND_RECEIVE,
    SEND_ONLY,
    RECEIVE_ONLY,
    INACTIVE
};

typedef enum streamDirection streamDirection;

enum mediaType {
    AUDIO,
    VIDEO,
    APPLICATION,
    TEXT,
    IMAGE,
    MESSAGE
};

typedef enum mediaType mediaType;

class sdpMedia
{
    public:
        sdpMedia( int type );
        sdpMedia( std::string media, int port );
        ~sdpMedia();

        /*
         * Read accessor. Return the list of codecs
         */
        std::vector<sdpCodec*> getMediaCodecList() { return _codecList; }

        /*
         * Read accessor. Return the type of media
         */
        mediaType getMediaType() { return _mediaType; }

        /*
         * Read accessor. Return the type of media
         */
        std::string getMediaTypeStr();

        /*
         * Read accessor. Return the transport port
         */
        int getPort() { return _port; }

        /*
         * Write accessor. Set the transport port
         */
        void setPort( int port ) { _port = port; }

        /*
         * Add a codec in the current media codecs vector
         *
         * @param codec     A pointer on the codec to add
         */
        void addCodec( sdpCodec *codec );

        void addCodec( std::string codecName );

        void removeCodec( std::string codecName );

        void clearCodecList( void );

        std::string toString( void );

        void setStreamDirection( int direction ) { _streamType = (streamDirection)direction; }

        streamDirection getStreamDirection( void ) { return _streamType; }

        std::string getStreamDirectionStr( void );

    private:
        /* The type of media */
        mediaType _mediaType;

        /* The media codec vector */
        std::vector< sdpCodec* > _codecList;

        /* the transport port */
        int _port;

        /* The stream direction */
        streamDirection _streamType;
};

#endif // _SDP_MEDIA
