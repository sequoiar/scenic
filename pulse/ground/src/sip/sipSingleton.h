#ifndef _SIP_SINGLETON_H_
#define _SIP_SINGLETON_H_

class SipSingleton
{
public:
    static SipSingleton* Instance();

    //called inside pjsip rx_request 
    //must return pchar to desired response
    char *rx_req(void *data, unsigned int len);
    
    //called inside pjsip rx_response
    void rx_res(void *data, unsigned int len);

private:
    SipSingleton(){};

    // Incoming data -> char* response
    static SipSingleton *s;
};

#endif
