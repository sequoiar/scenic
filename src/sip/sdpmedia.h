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

class sdpMedia {

    public:
        sdpMedia( int type );
        ~sdpMedia();

        std::vector<sdpCodec*> getMediaCodecList() { return _codecList; }
        int getType() { return _type; }
        int getPort() { return _t_port; }
        void setPort( int port ) { _t_port = port; }

        void addCodec( sdpCodec* codec );

    private:
        int _type;
        std::vector< sdpCodec* > _codecList;
        int _t_port;

};

#endif // _SDP_MEDIA
