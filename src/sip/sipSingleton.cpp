#include <iostream>
#include <sstream>

#include "sipSingleton.h"
#include "sipPrivate.h"

SipSingleton* SipSingleton::s = 0;

const char* SipSingleton::rx_req(const char *data, unsigned int len) 
{
    static char ser[16];
    static char p[8];
    std::cerr << "rx_request: " ;
    std::cerr.write(data, len);
    std::cerr << std::endl;
    sscanf(data,"%s",ser);

    strcpy(service,ser);

    std::cerr << ser << " port:" << port;

    sprintf(p,"%d",port);
    if(!strcmp(ser,"h264.1"))
        return p;

    if (!strncmp(data,"Hello",5))
    {
        return "Break Yourself!";
    }

    return "what?";
}



void SipSingleton::rx_res(const char *data, unsigned int len) 
{
    std::cerr << "rx_response:" ;
    std::cerr.write(data,len);
    std::cerr << std::endl;
}



SipSingleton* SipSingleton::Instance()
{
    if(s == 0)
        s = new SipSingleton();

    return s;
}

void SipSingleton::send_request(const char* msg)
{
    ::send_request(msg);

}

int SipSingleton::handle_events(void)
{
    return ::sip_handle_events();
}

bool SipSingleton::init(int argc, char* argv[])
{
    if(sip_pass_args(argc,argv) < 0)
        return false;

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





