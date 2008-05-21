#include <iostream>
#include <sstream>

#include "sipSingleton.h"
#include "sipPrivate.h"

SipSingleton* SipSingleton::s_ = 0;

const char* SipSingleton::rx_req(const char *data, unsigned int len) 
{
    static char ser[16];
    static char p[8];
    std::cerr << __FILE__ << ": rx_request: " ;
    std::cerr.write(data, len);
    std::cerr << std::endl;
    sscanf(data,"%s", ser);

    //std::string temp(ser);

    if (isValidService(std::string(ser)))
    {
        strcpy(service_, ser);

        std::cerr << ser << " port:" << service_port_ << std::endl;

        sprintf(p, "%d", service_port_);

        return p;
    }

    if (!strncmp(data,"Hello",5))
    {
        return "Break Yourself!";
    }

    return "what?";
}



bool SipSingleton::isValidService(std::string ser)
{
    if(!ser.compare("h264.1") || !ser.compare("dv") 
            || !ser.compare("test"))
        return true;
    else
        return false;
}

void SipSingleton::rx_res(const char *data, unsigned int len) 
{
    std::cerr << __FILE__ << ": rx_response:" ;
    std::cerr.write(data,len);
    std::cerr << std::endl;
    
    rx_port_ = atoi(data);
}



SipSingleton* SipSingleton::Instance()
{
    if(s_ == 0)
        s_ = new SipSingleton();

    return s_;
}



void SipSingleton::send_request(const char* msg)
{
    ::send_request(msg);
}



int SipSingleton::handle_events()
{
    return ::sip_handle_events();
}



bool SipSingleton::init(const char* local_port)
{
    sip_set_local(local_port);
    sip_init();
    return true;
}



bool SipSingleton::init(const char* local_ip,const char* local_port,
                        const char* remote_ip, const char* remote_port)
{
    sip_set_local(local_ip,local_port);
    sip_set_remote(remote_ip, remote_port);

    sip_init();
    return true;
}

