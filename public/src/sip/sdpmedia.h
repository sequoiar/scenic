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
        int getType() { return _type; }

        /*
         * Read accessor. Return the type of media
         */
        std::string getMediaStr() { return _mediaStr; }


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

    private:
        /* The type of media ( AUDIO/ VIDEO ..) */
        int _type;

        /* The media type, string form */
        std::string _mediaStr;

        /* The media codec vector */
        std::vector< sdpCodec* > _codecList;

        /* the tranport port */
        int _port;
};

#endif // _SDP_MEDIA
