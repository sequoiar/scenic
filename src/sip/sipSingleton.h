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

    void set_service_port(int p){port=p;}

    void zero_rx_port(){rx_port = 0;}

    int get_rx_port(){return rx_port;}
    int get_tx_port(){return port;}
    char* get_service(){return service;}
private:
    SipSingleton():port(0),rx_port(0){};
        

        void set_service_port(int p) { port_ = p; }

    private:
        SipSingleton(){};

        char service_[32];
        int port_,rx_port;

        static SipSingleton *s_;
};

#endif
