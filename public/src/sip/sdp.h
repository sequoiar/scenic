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

#include <vector>
#include <pjmedia/sdp.h>
#include <pjmedia/sdp_neg.h>
#include <pj/pool.h>
#include <pj/assert.h>

#include "sdpcodec.h"

class Sdp
{

	public:
		/* Class constructors */
		Sdp();
		Sdp( int aport, int vport );

		/* Class destructor */
		~Sdp();

		void addAudioCodec( sdpCodec* codec );
		void addVideoCodec( sdpCodec* codec );
		void setAudioPort( int aport ) { _audioPort = aport; }
		void setVideoPort( int vport ) { _videoPort = vport; }
		int getAudioPort( void ) { return _audioPort; }
		int getVideoPort( void ) { return _videoPort; }
		std::vector<sdpCodec*> getAudioCodecsList( void ) { return _audiocodecs; }
		std::vector<sdpCodec*> getVideoCodecList( void ) { return _videocodecs; }
        void setCodecsList( void );
        void displayCodecsList( void );
        pjmedia_sdp_session* getSDPSession( void ) { return _local_offer; }
        void createLocalOffer( pj_pool_t *pool );
        void toString( void );
        pjmedia_sdp_media* getMediaDescriptorLine( pj_pool_t* pool );
        void createInitialOffer( pj_pool_t* pool );
        void receivingInitialOffer( pj_pool_t* pool, pjmedia_sdp_session* remote );


	private:
		std::string _sdpBody;
		int _audioPort, _videoPort;
		std::vector<sdpCodec*> _audiocodecs;
		std::vector<sdpCodec*> _videocodecs;
        pjmedia_sdp_session *_local_offer;
        pjmedia_sdp_neg *negociator;

        Sdp(const Sdp&); //No Copy Constructor
        Sdp& operator=(const Sdp&); //No Assignment Operator

        bool audioMedia(){ return _audiocodecs.size() != 0; } 
        bool videoMedia(){ return _videocodecs.size() != 0; }
};

#endif // _SDP_H

