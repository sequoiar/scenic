// sdp.cpp
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
 *      Implemets SDP protocol
 *      Classes Sdp, SdpMedia, SdpMediaFactory
 */


#include "sdp.h"


bool Sdp::add_media(SdpMedia _m)
{

    if(_m.get_port() != 0)
        if (_m.get_ip().empty() && !ip.empty())
            _m.set_ip(ip);

    if (!_m.media_type.empty() && !_m.get_ip().empty() && _m.get_port() != 0)
        media.push_back(_m);
    else
        return 0;

    return 1;

}

SdpMedia& SdpMedia::operator=(const SdpMedia& m)
{ 
    if(&m == this)
        return *this;
    
    ip = m.ip;
    attrib = m.attrib;
    port = m.port;


    return *this;
}

std::string SdpMedia::str()
{
    std::stringstream ret;

    ret << "m=" << media_type << " " << port << " RTP/AVP " << avp_type << std::endl;
    if(!ip.empty())
        ret << "c=IN IP4 " << ip << std::endl;

    ret << "a=rtpmap:" << avp_type << " " << codec << std::endl;

    ret << "a=fmtp:" << avp_type << std::endl;

    for(std::list<std::string>::iterator it = attrib.begin();it != attrib.end(); it++)
        ret << "a=" << *it << std::endl;

    return ret.str();
}



std::string Sdp::str()
{
    std::stringstream ret;

    ret << "v=0" << std::endl;
    if(ip.empty())
        ret << "o=- 304958 309458 IN IP4 " << "127.0.0.0" << std::endl;
    else
        ret << "o=- 304958 309458 IN IP4 " << ip << std::endl;

    ret << "s=" << session_name << std::endl;

    if(!ip.empty())
        ret << "c=IN IP4 " << ip << std::endl;

    for(std::list<SdpMedia>::iterator it = media.begin();it != media.end(); it++)
        ret <<  it->str() << std::endl;

    return ret.str();

}
/*
void SdpMedia::add_attribute(std::string _attrib)
{
    attrib.push_back(_attrib);
}
*/

std::map<std::string,SdpMedia> SdpMediaFactory::_sdpMedia_prototypes;

const SdpMedia& SdpMediaFactory::clone(std::string s)
{
    if(_sdpMedia_prototypes.empty())
    {
       //RFC 3984  8.2.1
        _sdpMedia_prototypes["H264"] = SdpMedia("video","H264/90000",98);
        _sdpMedia_prototypes[""] = SdpMedia();
    }

    if(_sdpMedia_prototypes.find(s) != _sdpMedia_prototypes.end())
    {
        return _sdpMedia_prototypes[s];
    }

    return _sdpMedia_prototypes[""];
}

