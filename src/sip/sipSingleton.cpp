#include <iostream>
#include <sstream>

#include "sipSingleton.h"
#include "sipPrivate.h"

SipSingleton* SipSingleton::s = 0;

const char* SipSingleton::rx_req(const char *data, unsigned int len) 
{
    std::cerr << "rx_request: " ;
    std::cerr.write(data, len);
    std::cerr << std::endl;

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






int main(int argc, char *argv[])
{
    SipSingleton &sip = *SipSingleton::Instance();

    if(!sip.init(argc,argv))
        return -1;

    if (argc == 5)
    {
        sip.send_request("Hello World");
    }


    for (;;)
    {
        static int eventCount;
        if (eventCount += sip.handle_events())
        {
            std::cout << "HANDLED " << eventCount << " EVENTS " << std::endl;
        }
    }

    return 0;
}

