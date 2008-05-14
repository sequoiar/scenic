#ifndef _SIP_SINGLETON_H_
#define _SIP_SINGLETON_H_

class sip_singleton{
public:
    static sip_singleton* Instance();
    char *rx_req(void *data,unsigned int len );
    void rx_res(void *data,unsigned int len );
private:
    sip_singleton(){};


    // Incomming data -> char* response
    static sip_singleton *s;
};

#endif
