#ifndef _SDP_
#define _SDP_

#include <string>
#include <sstream>
#include <list>

#define MAX_MEDIA 2

class SdpMedia
{
public:
    SdpMedia(std::string _name, std::string _address, int _port, int _avp_type)
        :name(_name),address(_address),port(_port),num_attrib(0),avp_type(_avp_type)
    {}

    SdpMedia()
    {}

    void add_attribute(std::string _attrib);

    std::string str();
protected:
    std::string name,address;
    std::list<std::string> attrib;
    int port;
    int num_attrib;
    int avp_type;
};


class SdpVideo: public SdpMedia
{
public:
    SdpVideo(std::string _address, int _port)
        :SdpMedia("video",_address,_port,96)
        {}

private:

};




class Sdp
{
public:
    Sdp(std::string _session_name,std::string _address)
        :session_name(_session_name),address(_address),num_media(0)
        {}

    void add_media(SdpMedia _m);
    std::string str();

private:
    std::string session_name, address;

    std::list<SdpMedia> media;
    int num_media;
};

#endif

