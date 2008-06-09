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
 *      Implements SDP protocol
 *      Classes Sdp, SdpMedia, SdpMediaFactory
 */


#include <iostream>
#include "sdp.h"


bool Sdp::add_media(SdpMedia m) 
{
    if(m.get_port() != 0)
        if (m.get_ip().empty() && !ip_.empty())
            m.set_ip(ip_);


    if (!m.get_media_type().empty() && !m.get_ip().empty() && m.get_port() != 0)
        media_.push_back(m);
    else
        return 0;

    return 1;
}
/*


SdpMedia& SdpMedia::operator=(const SdpMedia& m)
{ 
    if(&m == this)
        return *this;
    
    ip_ = m.ip_;
    attrib_ = m.attrib_;
    port_ = m.port_;


    return *this;
}
*/



std::string SdpMedia::str()
{
    std::stringstream ret;

    ret << "m=" << media_type_ << " " << port_ << " RTP/AVP " << avp_type_ << std::endl;
    if(!ip_.empty())
        ret << "c=IN IP4 " << ip_ << std::endl;

    ret << "a=rtpmap:" << avp_type_ << " " << codec_ << std::endl;

    ret << "a=fmtp:" << avp_type_ << std::endl;

    for(std::vector<std::string>::iterator it = attrib_.begin(); it != attrib_.end(); it++)
        ret << "a=" << *it << std::endl;

    return ret.str();
}



std::string Sdp::str()
{
    std::stringstream ret;

    ret << "v=0" << std::endl;

    if(ip_.empty())
        ret << "o=- 304958 309458 IN IP4 " << "127.0.0.0" << std::endl;
    else
        ret << "o=- 304958 309458 IN IP4 " << ip_ << std::endl;

    ret << "s=" << session_name_ << std::endl;

    if(!ip_.empty())
        ret << "c=IN IP4 " << ip_ << std::endl;

    for(std::vector<SdpMedia>::iterator it = media_.begin(); it != media_.end(); it++)
        ret << it->str() << std::endl;

    return ret.str();
}



void Sdp::list_media()
{
    for(std::vector<SdpMedia>::iterator it = media_.begin(); it != media_.end(); it++)
    {
        std::cout << "Got media: " << it->get_media_type() << std::endl;
    }
}


std::vector<SdpMedia>& Sdp::get_media()  
{
    return media_;
}


/*
void SdpMedia::add_attribute(std::string _attrib)
{
    attrib.push_back(_attrib);
}
*/

std::map<std::string, SdpMedia> SdpMediaFactory::sdpMedia_prototypes_;

const SdpMedia& SdpMediaFactory::clone(std::string s)
{
    if(sdpMedia_prototypes_.empty())
    {
       //RFC 3984  8.2.1
        sdpMedia_prototypes_["H264"] = SdpMedia("video","H264/90000",98);
        sdpMedia_prototypes_[""] = SdpMedia();
    }

    if(sdpMedia_prototypes_.find(s) != sdpMedia_prototypes_.end())
    {
        return sdpMedia_prototypes_[s];
    }

    return sdpMedia_prototypes_[""];
}

