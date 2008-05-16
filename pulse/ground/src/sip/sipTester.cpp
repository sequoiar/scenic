
#include "sipTester.h"

SipTester::SipTester(SipSingleton &sip) : sip(sip)
{
}  


SipTester::~SipTester()
{
    // empty
}



void SipTester::create_session()
{
    sip.set_service_port(10010);
    sip.init("192.168.1.164","5060","192.168.1.164","5061");
}
 



void SipTester::send_messages()
{
    sip.send_request("h264.1");
    sip.send_request("blah");
}
