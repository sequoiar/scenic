// sdp.h
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

/**  \file 
 *  Incapsulate SDP protocol.
 *  Reads and Writes SDP protocol.\n 
 *  
 *  
 *  TODO - Currently only writes SDP 
 */


#ifndef _SDP_
#define _SDP_

#include <string>
#include <sstream>
#include <list>
#include <map>
#include <cassert>

class SdpMedia;
/** Incapsulates the sdp protocol. 
 *
 *  Valid Sdp class has one or many SdpMedia objects.\n
 *   
 *  TODO Parse SDP stream
 */
class Sdp
{
public:
    Sdp(std::string _session_name = "<No Title>")
        :session_name(_session_name)
        {}

    bool add_media(SdpMedia _m);
    std::string str();

    std::list<SdpMedia>& get_media() {return media;}
    bool is_valid(){ return !media.empty();}
private:
    std::string session_name, ip;

    std::list<SdpMedia> media;
};

/** Holds the media section of sdp protocol 
 *
 *  SdpMedia allows access to ip ports media codec attribs etc.
 *  See SdpMediaFactory for various prototypes for media
 */
class SdpMedia
{
public:
    SdpMedia(std::string _media_type, std::string _codec, int _avp_type)
        :media_type(_media_type),codec(_codec),avp_type(_avp_type) {}

    SdpMedia():media_type(""),codec(""),avp_type(0) {}

    std::string media_type,codec;
    int avp_type;

    void add_attribute(std::string _attrib){attrib.push_back(_attrib);}
    void set_ip(std::string _ip){ip = _ip;}
    void set_port(int _port){port = _port;}
    int get_port(){return port;}
    std::string& get_ip(){return ip;} 
    std::string str();
 //   SdpMedia& operator=(const SdpMedia& m);
protected:
    std::string ip;
    std::list<std::string> attrib;
    int port;
};

/** use clone to get a copy of various SdpMedia prototypes */
class SdpMediaFactory
{
public:
    static const SdpMedia& clone(std::string);
    
private:
    static std::map<std::string,SdpMedia> _sdpMedia_prototypes;
};



#endif

