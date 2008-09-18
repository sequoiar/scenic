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

#include "instantmessaging.h"

InstantMessaging::InstantMessaging()
    : _current_dlg( NULL ), _message(EMPTY_MESSAGE), _response(EMPTY_MESSAGE){}


InstantMessaging::~InstantMessaging(){
    delete _current_dlg; _current_dlg = 0;
}


void InstantMessaging::setText( std::string message ){
    _message = pj_str((char*)message.c_str());
}


void InstantMessaging::setResponse( std::string resp ){
    _response = pj_str((char*)resp.c_str());
}


std::string InstantMessaging::getTextMessage( void ){
    std::string text;

    text = _response.ptr;
    return text;
}

pj_status_t InstantMessaging::sendMessage(){
    pjsip_method msg_method;
    const pj_str_t type =  STR_TEXT;
    const pj_str_t subtype = STR_PLAIN;
    pjsip_tx_data *tdata;
    pj_status_t status;

    msg_method.id = PJSIP_OTHER_METHOD;
    msg_method.name = METHOD_NAME ;

    // Must lock dialog
    pjsip_dlg_inc_lock( _current_dlg );

    // Create the message request
    status = pjsip_dlg_create_request( _current_dlg, &msg_method, -1 /*CSeq*/, &tdata );
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Attach "text/plain" body */
    tdata->msg->body = pjsip_msg_body_create( tdata->pool, &type, &subtype, &_message );

    // Send the request
    status = pjsip_dlg_send_request( _current_dlg, tdata, -1, NULL);
    PJ_ASSERT_RETURN( status == PJ_SUCCESS, 1 );

    // Done
    pjsip_dlg_dec_lock( _current_dlg );

    return PJ_SUCCESS;
}


void InstantMessaging::displayResponse( void ){
    std::cout << "<IM> " << _response.ptr << std::endl;
}

