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

#include "uri.h"
#include "hostIP.h"

#include <pjlib.h>
#include <sstream>
#include <iostream>

URI::URI( int port )
    : _host_name(""), _host_ip(""), _port(0) {
    // Guess the local parameters
    _host_ip = get_host_ip();
    _host_name = pj_gethostname()->ptr;
    _port = port;
}


URI::URI( std::string uri )
    : _host_name(""), _host_ip(""), _port(0) {
    using std::cout;
    using std::endl;
    size_t pos;
    std::string tmp;
    tmp = strdup(uri.c_str());
    // Build a sip URI object with the entire address
    // Pattern : <sip:username@hostip:port>
    //
    // First: Remove the unuseful stuff, ie '<sip:
    pos = tmp.find(":", 0);
    tmp.erase(0, pos+1);
    // and '>'
    tmp.erase( tmp.length() -1, tmp.length()-1);

    // Retrieve the username
    pos = tmp.find("@", 0);
    _host_name = tmp.substr(0, pos);
    tmp.erase(0, pos+1);

    // Retrieve the host IP
    pos = tmp.find(":", 0);
    _host_ip = tmp.substr(0, pos);
    tmp.erase(0, pos+1);

    // Finally the port number
    _port = atoi(tmp.c_str());
}


URI::URI( std::string hostname, std::string hostip, int port )
    : _host_name(hostname), _host_ip(hostip), _port(port) {}

std::string URI::get_address(){
    std::ostringstream addr;

    addr << "<sip:" << get_host_name() << "@" << get_hostip() << ":" << get_port() << ">";

    return addr.str();
}


void URI::to_string(){
    using std::cout;
    using std::endl;

    cout << "Host name = " << get_host_name() << endl;
    cout << "Host IP = " << get_hostip() << endl;
    cout << "Port = " << get_port() << endl;
    cout << "<sip:" << get_host_name() << "@" << get_hostip() << ":" << get_port() << ">" << endl;
}


/**************************** PRIVATE MEMBERS **************************************/
