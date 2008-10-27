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
         * Read accessor. Get the list of the local media capabilities. 
         *
         * @return std::vector<sdpMedia*>   the vector containing the different media
         */
        std::vector<sdpMedia*> get_local_media_cap( void ) { return _local_media_cap; }

        /*
         *  Read accessor. Get the sdp session information
         *
         *  @return pjmedia_sdp_session   The structure that describes a SDP session
         */
        pjmedia_sdp_session* get_local_sdp_session( void ) { return _local_offer; }

        /*
         * Write accessor. Set the local IP address that will be used in the sdp session
         */
        void set_ip_address( std::string ip_addr ) { _ip_addr = ip_addr; }
        
        /*
         * Read accessor. Get the local IP address
         */
        std::string get_ip_address( void ) { return _ip_addr; }

        /*
         * Build the local SDP offer
         */
        int create_local_offer( );

        /*
         * Build the sdp media section
         * Add rtpmap field if necessary
         *
         * @param media     The media to add to SDP
         * @param med   The structure to receive the media section
         */
        void set_media_descriptor_line( sdpMedia* media, pjmedia_sdp_media** p_med );

        /*
         * On building an invite outside a dialog, build the local offer and create the
         * SDP negociator instance with it.
         */
        int create_initial_offer( );

        /*
         * On receiving an invite outside a dialog, build the local offer and create the
         * SDP negociator instance with the remote offer.
         *
         * @param remote    The remote offer
         */
        int receiving_initial_offer( pjmedia_sdp_session* remote );

        /*
         * Set the local media capablities. Add a media in the session 
         *
         * @param mime_type The type of media
         * @param codecs    The formatted list of codecs name (separator: '/')
         */
        void set_local_media_cap( std::string type, std::string codecs, int port, std::string dir );

        /*
         * Start the sdp negociation.
         *
         * @return pj_status_t  0 on success
         *                      1 otherwise
         */
        pj_status_t start_negociation( void ){
            return pjmedia_sdp_neg_negotiate(
                       _pool, negociator, 0);
        }

        /*
         * Retrieve the negociated sdp offer from the sip payload.
         *
         * @param sdp   the negociated offer
         */
        void set_negociated_offer( const pjmedia_sdp_session *sdp );

        /*
         * Remove all media in the session media vector.
         */
        void clean_session_media();

        /*
         * read accessor. Return the negociated offer
         *
         * @return pjmedia_sdp_session  The negociated offer
         */
        pjmedia_sdp_session* get_negociated_offer( void ){
            return _negociated_offer;
        }

        /*
         * Return a string description of the media added to the session,
         * ie the local media capabilities
         */
        std::string media_to_string( void );

        /*
         * Return the codec of the first media after negociation
         */
        std::string get_session_media( void );

    private:

        // The local media capabilities 
        std::vector<sdpMedia*> _local_media_cap;
    
        // The media that will be used by the session (after the SDP negociation)
        std::vector<sdpMedia*> _session_media;

        /* The local IP address */
        std::string _ip_addr;

        /* The local SDP offer */
        pjmedia_sdp_session *_local_offer;

        /* The negociated SDP offer */
        // Explanation: each endpoint's offer is negociated, and a new sdp offer results from this
        // negociation, with the compatible media from each part 
        pjmedia_sdp_session *_negociated_offer;

        /* The sdp negociator instance */
        pjmedia_sdp_neg *negociator;
    
        // The pool to allocate memory
        pj_pool_t *_pool;

        Sdp(const Sdp&); //No Copy Constructor
        Sdp& operator=(const Sdp&); //No Assignment Operator

        /*
         *  Mandatory field: Protocol version ("v=")
         *  Add the protocol version in the SDP session description
         */
        void sdp_add_protocol( void );

        /*
         *  Mandatory field: Origin ("o=")
         *  Gives the originator of the session.
         *  Serves as a globally unique identifier for this version of this session description.
         */
        void sdp_add_origin( void );

        /*
         *  Mandatory field: Session name ("s=")
         *  Add a textual session name.
         */
        void sdp_add_session_name( void );

        /*
         *  Optional field: Session information ("s=")
         *  Provides textual information about the session.
         */
        void sdp_add_session_info( void ){}

        /*
         *  Optional field: Uri ("u=")
         *  Add a pointer to additional information about the session.
         */
        void sdp_add_uri( void ) {}

        /*
         *  Optional fields: Email address and phone number ("e=" and "p=")
         *  Add contact information for the person responsible for the conference.
         */
        void sdp_add_email( void ) {}

        /*
         *  Optional field: Connection data ("c=")
         *  Contains connection data.
         */
        void sdp_add_connection_info( void );

        /*
         *  Optional field: Bandwidth ("b=")
         *  Denotes the proposed bandwidth to be used by the session or the media .
         */
        void sdp_add_bandwidth( void ) {}

        /*
         *  Mandatory field: Timing ("t=")
         *  Specify the start and the stop time for a session.
         */
        void sdp_add_timing( void );

        /*
         * Optional field: Time zones ("z=")
         */
        void sdp_add_time_zone( void ) {}

        /*
         * Optional field: Encryption keys ("k=")
         */
        void sdp_add_encryption_key( void ) {}

        /*
         * Optional field: Attributes ("a=")
         */
        void sdp_add_attributes( );

        /*
         * Mandatory field: Media descriptions ("m=")
         */
        void sdp_add_media_description(  );
};

#endif // _SDP_H

