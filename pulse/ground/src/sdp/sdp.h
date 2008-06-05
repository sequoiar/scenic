#ifndef _SDP_
#define _SDP_

#include <string>
#include <sstream>
#include <list>
#include <map>


class SdpMedia
{
public:
    SdpMedia(std::string _name, std::string _codec, int _avp_type)
        :name(_name),codec(_codec),num_attrib(0),avp_type(_avp_type)
    {}
    SdpMedia(){}

    void add_attribute(std::string _attrib){attrib.push_back(_attrib);}
    void set_ip(std::string _address){address = _address;}
    void set_port(int _port){port = _port;}
    int get_port(){return port;}
    std::string& get_ip(){return address;} 
    std::string str();
protected:
    std::string name,address,codec;
    std::list<std::string> attrib;
    int port;
    int num_attrib;
    int avp_type;
};


class SdpMediaFactory
{
public:
    static const SdpMedia& clone(std::string);
    
private:
    static std::map<std::string,SdpMedia> _sdpMedia_prototypes;
};

/*
class SdpVideo: public SdpMedia
{
public:
    SdpVideo(std::string _address, int _port)
        :SdpMedia("video",_address,_port,96)
        {}

private:

};
*/



class Sdp
{
public:
    Sdp(std::string _session_name,std::string _address = "127.0.0.0")
        :session_name(_session_name),address(_address),num_media(0)
        {}

    bool add_media(SdpMedia _m);
    std::string str();

private:
    std::string session_name, address;

    std::list<SdpMedia> media;
    int num_media;
};

#endif

