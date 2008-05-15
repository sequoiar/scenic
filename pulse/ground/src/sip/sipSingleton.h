#ifndef _SIP_SINGLETON_H_
#define _SIP_SINGLETON_H_

class SipSingleton
{
public:
    static SipSingleton* Instance();

    //called inside pjsip rx_request 
    //must return pchar to desired response
    const char *rx_req(const char *msg, unsigned int len);
    
    //called inside pjsip rx_response
    void rx_res(const char *msg, unsigned int len);

    void send_request(const char* msg);

    int handle_events(void);

    bool init(int argc, char* argv[]);


private:
    SipSingleton(){};

    // Incoming data -> char* response
    static SipSingleton *s;
};

#endif
