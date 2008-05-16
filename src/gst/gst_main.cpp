
// main_entry.c
#include "../sip/sipSingleton.h"
#include "../sip/sipTester.h"



int gst_main(int argc, char *argv[]);
    

int main(int argc, char *argv[])
{
    SipSingleton &sip = *SipSingleton::Instance();
    
    // SipTester client(sip);
    // client.create_session();
    // client.send_messages();
    sip.set_service_port(10010);

    sip.init("192.168.1.164","5060","192.168.1.164","5061");

    sip.send_request("h264.1");


    return gst_main(argc,argv);
}

