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
#include "sdpmedia.h"

/*
 * @file    sdp.h
 * @brief   A class to implement the Session Description Protocol.
 *          Thsi class build the SDP body and takes care of SDP negociation
 */

class Sdp
{
    public:

        /*
         * Class Constructor.
         *
         * @param ip_addr
         */
        Sdp(pj_pool_t *pool );

        /* Class destructor */
        ~Sdp();

        /*
         * Read accessor. Get the list of media
         *
         * @return std::vector<sdpCodec*>   the vector containing the audio codecs
         */
        std::vector<sdpMedia*> getSDPMediaList( void ) { return _sdpMediaList; }

        /*
         *  Read accessor. Get the sdp session information
         *
         *  @return pjmedia_sdp_session   The structure that describes a SDP session
         */
        pjmedia_sdp_session* getLocalSDPSession( void ) { return _local_offer; }

        void setIPAddress( std::string ip_addr ) { _ip_addr = ip_addr; }
        std::string getIPAddress( void ) { return _ip_addr; }

        /*
         * Build the local SDP offer
         *
         * @param pool  The pool to allocate memory
         */
        int createLocalOffer( );

        /*
         * Build the sdp media section
         * Add rtpmap field if necessary
         *
         * @param media     The media to add to SDP
         * @param pool  The pool to allocate memory
         * @param med   The structure to receive the media section
         */
        void getMediaDescriptorLine( sdpMedia* media, pjmedia_sdp_media** p_med );

        /*
         * On building an invite outside a dialog, build the local offer and create the
         * SDP negociator instance with it.
         *
         * @param pool  The pool to allocate memory
         */
        int createInitialOffer( );

        /*
         * On receiving an invite outside a dialog, build the local offer and create the
         * SDP negociator instance with the remote offer.
         *
         * @param pool  The pool to allocate memory
         * @param remote    The remote offer
         */

        int receivingInitialOffer( pjmedia_sdp_session* remote );

        /*
         * Parse a list of formatted encoding codecs name and add it to the session media
         *
         * @param mime_type The type of media
         * @param codecs    The formatted list of codecs name (separator: '/')
         */
        void setSDPMedia( std::string type, std::string codecs, int port, std::string dir );

        pj_status_t startNegociation(  ){
            return pjmedia_sdp_neg_negotiate(
                       _pool, negociator, 0);
        }


        std::string mediaToString( void );

    private:

        /* The media list */
        std::vector<sdpMedia*> _sdpMediaList;

        /* The local IP address */
        std::string _ip_addr;

        /* The local SDP offer */
        pjmedia_sdp_session *_local_offer;

        /* The sdp negociator instance */
        pjmedia_sdp_neg *negociator;

        pj_pool_t *_pool;

        Sdp(const Sdp&); //No Copy Constructor
        Sdp& operator=(const Sdp&); //No Assignment Operator

        /*
         *  Mandatory field: Protocol version ("v=")
         *  Add the protocol version in the SDP session description
         */
        void sdp_addProtocol( void );

        /*
         *  Mandatory field: Origin ("o=")
         *  Gives the originator of the session.
         *  Serves as a globally unique identifier for this version of this session description.
         */
        void sdp_addOrigin( void );

        /*
         *  Mandatory field: Session name ("s=")
         *  Add a textual session name.
         */
        void sdp_addSessionName( void );

        /*
         *  Optional field: Session information ("s=")
         *  Provides textual information about the session.
         */
        void sdp_addSessionInfo( void ){}

        /*
         *  Optional field: Uri ("u=")
         *  Add a pointer to additional information about the session.
         */
        void sdp_addUri( void ) {}

        /*
         *  Optional fields: Email address and phone number ("e=" and "p=")
         *  Add contact information for the person responsible for the conference.
         */
        void sdp_addEmail( void ) {}

        /*
         *  Optional field: Connection data ("c=")
         *  Contains connection data.
         */
        void sdp_addConnectionInfo( void );

        /*
         *  Optional field: Bandwidth ("b=")
         *  Denotes the proposed bandwidth to be used by the session or the media .
         */
        void sdp_addBandwidth( void ) {}

        /*
         *  Mandatory field: Timing ("t=")
         *  Specify the start and the stop time for a session.
         */
        void sdp_addTiming( void );

        /*
         * Optional field: Time zones ("z=")
         */
        void sdp_addTimeZone( void ) {}

        /*
         * Optional field: Encryption keys ("k=")
         */
        void sdp_addEncryptionKey( void ) {}

        /*
         * Optional field: Attributes ("a=")
         */
        void sdp_addAttributes( );

        /*
         * Mandatory field: Media descriptions ("m=")
         */
        void sdp_addMediaDescription(  );
};

#endif // _SDP_H

