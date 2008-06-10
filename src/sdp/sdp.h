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
 *  Encapsulate SDP protocol.
 *  Reads and Writes SDP protocol.\n 
 *  
 *  
 *  TODO - Currently only writes SDP 
 */


#ifndef _SDP_
#define _SDP_

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cassert>

class SdpMedia;
/** Encapsulates the sdp protocol. 
 *
 *  Valid Sdp class has one or many SdpMedia objects.   \n
 *  SdpMediaFactory delivers SdpMedia prototypes.       \n
 *  TODO Parse SDP stream
 */

typedef std::vector<SdpMedia>::const_iterator SdpMediaIterator;

class Sdp
{
public:
    Sdp(std::string session_name = "<No Title>")
        :session_name_(session_name)
        {}

    bool add_media(SdpMedia m);
    std::string str();

    std::vector<SdpMedia>::const_iterator get_media_begin() {return media_.begin();}
    std::vector<SdpMedia>::const_iterator get_media_end() {return media_.end();}


    bool is_valid() const { return !media_.empty(); }
   
    bool parse(std::string);
    
    void list_media();

private:
    std::string session_name_, ip_;

    std::vector<SdpMedia> media_;
};

/** Holds the media section of sdp protocol 
 *
 *  SdpMedia allows access to ip ports media codec attribs etc.
 *  See SdpMediaFactory for various prototypes for media
 */
class SdpMedia
{
public:
    SdpMedia(std::string media_type, std::string codec, int avp_type)
        : media_type_(media_type), codec_(codec), avp_type_(avp_type) {}

    SdpMedia():media_type_(""), codec_(""), avp_type_(0) {}


    void add_attribute(std::string attrib) { attrib_.push_back(attrib); }
    void set_ip(std::string ip) {ip_ = ip;}
    void set_port(int port) {port_ = port;}
    int get_port() const { return port_; }
    const std::string& get_ip() const { return ip_; }  
    const std::string& get_media_type() const { return media_type_; }
    std::string str();
    //SdpMedia& operator=(const SdpMedia& m);
protected:
    std::string media_type_, codec_;
    int avp_type_;
    std::string ip_;
    std::vector<std::string> attrib_;
    int port_;
};

/** use clone to get a copy of various SdpMedia prototypes */
class SdpMediaFactory
{
public:
    static const SdpMedia& clone(std::string);
    
private:
    static std::map<std::string, SdpMedia> sdpMedia_prototypes_;
};



#endif

