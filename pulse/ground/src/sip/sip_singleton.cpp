#include "sip_singleton.h"



char* sip_singleton::rx_req(void *data,unsigned int len ) 
{
    return 0;
}

void sip_singleton::rx_res(void *data,unsigned int len ) {}

sip_singleton* sip_singleton::s = 0;
sip_singleton* sip_singleton::Instance()
{
    if(s == 0)
        s = new sip_singleton();

    return s;

}


