// sip.h
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
 *      Implements SIP protocol on IP
 *
 */

#ifndef _SIP_SINGLETON_H_
#define _SIP_SINGLETON_H_

#include <string>

#include "sdp/sdp.h"

class SipSingleton
{

public:
static SipSingleton *Instance();

//called inside pjsip rx_request
//must return pchar to desired response
// Incoming data -> char* response
const char *rx_invite(const char *msg, unsigned int len);

//called inside pjsip rx_response
void rx_res(const char *msg, unsigned int len);

void send_request(std::string);

int handle_events();

bool init(const char *local_port);

bool init(const char *local_ip, const char *local_port,
          const char *remote_ip, const char *remote_port);

bool isValidService();

void set_service_port(int p)
{
	service_port_ = p;
}
int get_service_port()
{
	return service_port_;
}

void zero_service_port()
{
	service_port_ = 0;
}
void zero_service_desc()
{
	service_[0] = 0;
}

int get_rx_port()
{
	return rx_port_;
}
void zero_rx_port()
{
	rx_port_ = 0;
}
char *get_service()
{
	return service_;
}

bool response_ok()
{
	return response;
}
Sdp & get_sdp()
{
	return sdp_;
}
private:
SipSingleton() : service_port_(0), rx_port_(0), response(false)
{
};

char service_[32];
int service_port_, rx_port_;

bool response;

Sdp sdp_;

static SipSingleton *s_;
};

#endif
