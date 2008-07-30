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

#ifndef _SDP_H
#define _SDP_H

#include <list>
#include <pjmedia/sdp.h>
#include <pjmedia/sdp_neg.h>

#include "sdpcodec.h"

class Sdp
{
    public:
/* Class constructors */
        Sdp();
        Sdp( int aport, int vport );

/* Class desctructor */
        ~Sdp();

        void addAudioCodec( sdpCodec* codec );

        void addVideoCodec( sdpCodec* codec );

        void setAudioPort( int aport );

        void setVideoPort( int vport );

        int getAudioPort( void ) { return _audioPort; }

        int getVideoPort( void ) { return _videoPort; }

        std::list<sdpCodec> getAudiocodecsList( void ) { return _audiocodecs; }

        std::list<sdpCodec> getVideocodecList( void ) { return _videocodecs; }

    private:
        std::string _sdpBody;

        std::list<sdpCodec> _audiocodecs;
        std::list<sdpCodec> _videocodecs;

        int _audioPort, _videoPort;
};

#endif // _SDP_H

