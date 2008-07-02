// sip.cpp
// Copyright 2008 Koya Charles & Tristan Matthews 
//     
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

/** \file 
 *      Implements the SIP protocol with IP 
 *
 *      Contains the class SipSingleton
 *      
 */

#include <iostream>

#include <sstream>

#include "sip.h"
#include "sipPrivate.h"

SipSingleton *SipSingleton::s_ = 0;

#include "sdp/sdp.h"
#include "defaultAddresses.h"

const char *SipSingleton::rx_invite(const char *data, unsigned int len)
{
    std::cerr << __FILE__ << ": rx_request: ";
    std::cerr.write(data, len);
    std::cerr << std::endl;
//    sscanf(data,"%4s", ser);

    sdp_.parse(std::string(data, len));
    //std::string temp(ser);
/*
    SdpMedia sdpv = SdpMediaFactory::clone(ser);
    sdpv.set_ip(MY_ADDRESS);
    sdpv.set_port(service_port_);

    std::cout << sdp.str();

    sdp.add_media(sdpv);
*/
    std::cout << sdp_.str();

    std::cout << "-----------" << std::endl;

    if (sdp_.is_valid())
    {
        //  strcpy(service_, ser);

        std::cerr << sdp_.get_media_begin()->get_ip() << " port:"
            << sdp_.get_media_begin()->get_port() << std::endl;

        // sprintf(p, "%d", service_port_);

        return "OK";
    }

    if (!strncmp(data, "Hello", 5))
    {
        return "Break Yourself!";
    }

    return "what?";
}

bool SipSingleton::isValidService()
{
    if (sdp_.is_valid())
        return true;
    return false;
}

void SipSingleton::rx_res(const char *data, unsigned int len)
{

    std::cerr << __FILE__ << ": rx_response:";
    std::cerr.write(data, len);
    std::cerr << std::endl;

    if (!strncmp(data, "OK", 2))
        response = true;

}

SipSingleton *SipSingleton::Instance()
{
    if (s_ == 0)
        s_ = new SipSingleton();

    return s_;
}

void SipSingleton::send_request(std::string msg)
{
    ::send_request(msg.c_str());
}

int SipSingleton::handle_events()
{
    return::sip_handle_events();
}

bool SipSingleton::init(const char *local_port)
{
    sip_set_local(local_port);
    sip_init();
    return true;
}

bool SipSingleton::init(const char *local_ip, const char *local_port,
                        const char *remote_ip, const char *remote_port)
{
    sip_set_local(local_ip, local_port);
    sip_set_remote(remote_ip, remote_port);

    sip_init();
    return true;
}
