#include "sdp.h"


void Sdp::add_media(SdpMedia _m)
{
    media.push_back(_m);
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

    if(!address.empty())
    ret << "c=IN IP4 " << address << std::endl;

    for(std::list<SdpMedia>::iterator it = media.begin();it != media.end(); it++)
        ret <<  it->str() << std::endl;

    return ret.str();

}

void SdpMedia::add_attribute(std::string _attrib)
{
    attrib.push_back(_attrib);
}


