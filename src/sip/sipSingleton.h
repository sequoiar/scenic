#ifndef _SIP_SINGLETON_H_
#define _SIP_SINGLETON_H_

class SipSingleton
{
public:
    static SipSingleton* Instance();

    //called inside pjsip rx_request 
    //must return pchar to desired response
    // Incoming data -> char* response
    const char *rx_req(const char *msg, unsigned int len);

    //called inside pjsip rx_response
    void rx_res(const char *msg, unsigned int len);

    void send_request(const char* msg);

    int handle_events(void);

    bool init(const char* local_port);

        bool init(const char* local_ip,const char* local_port,
                const char* remote_ip, const char* remote_port);

    void set_service_port(int p){service_port_=p;}
    int get_service_port(){return service_port_;}

    void zero_service_port(){service_port_ = 0;}
    void zero_service_desc(){service_[0] = 0;}

    int get_rx_port(){return rx_port_;}
    void zero_rx_port(){rx_port_ = 0;}
    char* get_service(){return service_;}
private:
    SipSingleton():service_port_(0),rx_port_(0){};
        
    char service_[32];
    int service_port_,rx_port_;

    static SipSingleton *s_;
};

#endif
