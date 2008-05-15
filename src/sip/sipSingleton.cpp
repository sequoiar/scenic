#include <iostream>
#include <sstream>
#include "sipSingleton.h"

SipSingleton* SipSingleton::s = 0;

using namespace std;

const char* SipSingleton::rx_req(char *data, unsigned int len) 
{
    cerr << "rx_request: " ;
    cerr.write(data,len);
    cerr <<endl;

    if (!strncmp(data,"Hello",5))
    {
        return "Yourself";
    }
    return "what?";
}



void SipSingleton::rx_res(char *data, unsigned int len ) 
{
    std::cerr << "rx_response:" ;
    cerr.write(data,len);
    cerr<< endl;
 
}



SipSingleton* SipSingleton::Instance()
{
    if(s == 0)
        s = new SipSingleton();

    return s;
}

