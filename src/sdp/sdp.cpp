#include "sdp.h"


bool Sdp::add_media(SdpMedia _m)
{
    if (!_m.get_ip().empty() && _m.get_port() != 0)
        media.push_back(_m);
    else
        return 0;

    return 1;

}

std::string SdpMedia::str()
{
    std::stringstream ret;

    ret << "m=" << name << " " << port << " RTP/AVP " << avp_type << std::endl;
    if(!address.empty())
        ret << "c=IN IP4 " << address << std::endl;

    for(std::list<std::string>::iterator it = attrib.begin();it != attrib.end(); it++)
        ret << "a=" << *it << std::endl;

    return ret.str();
}



std::string Sdp::str()
{
    std::stringstream ret;

    ret << "v=0" << std::endl;
    ret << "o=- 304958 309458 IN IP4 " << address << std::endl;
    ret << "s=" << session_name << std::endl;

    if(address != "127.0.0.0")
        ret << "c=IN IP4 " << address << std::endl;

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

