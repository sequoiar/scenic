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

#ifndef _INSTANT_MESSAGING_H
#define _INSTANT_MESSAGING_H

#include <string>
#include <iostream>
#include <pjsip.h>

#define EMPTY_MESSAGE   pj_str((char*)"")
#define STR_TEXT        pj_str((char*)"text")
#define STR_PLAIN       pj_str((char*)"plain")
#define METHOD_NAME     pj_str((char*)"MESSAGE")

class InstantMessaging
{
    public:
        /*
         * Class constructor
         */
        InstantMessaging();
        
        /*
         * Class destructor
         */
        ~InstantMessaging();

        /*
         * Attach the instant messaging module to an existing SIP dialog
         *
         * @param dlg   A pointer on the current pjsip_dialog structure
         */
        void setDialog( pjsip_dialog *dlg ) { _current_dlg = dlg; }
        
        /*
         * Prepare a string to be sent. This method have to be called
         * before sending each message
         *
         * @param message   The text message
         */
        void setText( std::string message );
        
        std::string getTextMessage(void) ; 

        /*
         * Send the message, previously set with the setText method, to the connected peer
         *
         * @return pj_status_t  0 on success
         *                      1 otherwise
         */
        pj_status_t sendMessage( void );
            
        /*
         * Set the response.
         *
         * @param resp    The last string message received
         */ 
        void setResponse( std::string resp );
        
        /*
         * Display the response
         */
        void displayResponse( void );

    private:

        /*
         * The pjsip_dialog instance through which the instant messaging module exists
         */
        pjsip_dialog *_current_dlg;
        
        /*
         * The message to be sent
         */
        pj_str_t _message;
        
        /*
         * The last response
         */
        pj_str_t _response;

        InstantMessaging(const InstantMessaging&); //No Copy Constructor
        InstantMessaging& operator=(const InstantMessaging&); //No Assignment Operator
};

#endif // _INSTANT_MESSAGING_H
