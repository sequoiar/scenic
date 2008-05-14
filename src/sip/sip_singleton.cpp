#include "sip_singleton.h"



char* SipSingleton::rx_req(void *data, unsigned int len) 
{
    return 0;
}



void SipSingleton::rx_res(void *data, unsigned int len ) 
{
    // empty
}



SipSingleton* SipSingleton::s = 0;



SipSingleton* SipSingleton::Instance()
{
    if(s == 0)
        s = new SipSingleton();

    return s;
}

