#include <iostream>
#include <sstream>

#include "sipSingleton.h"

SipSingleton* SipSingleton::s = 0;

const char* SipSingleton::rx_req(char *data, unsigned int len) 
{
    //std::cerr << "__THIS_FILE__";
    std::cerr << "rx_request: " ;
    std::cerr.write(data, len);
    std::cerr << std::endl;

    if (!strncmp(data,"Hello",5))
    {
        return "Yourself";
    }

    return "what?";
}



void SipSingleton::rx_res(char *data, unsigned int len) 
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

