
#include "sipTester.h"

SipTester::SipTester(SipSingleton &sip) : sip(sip)
{
}  


SipTester::~SipTester()
{
    // empty
}



void SipTester::create_req_session()
{
    sip.set_service_port(10010);
    sip.init("192.168.1.183","5060","192.168.1.183","5061");
}
 


void SipTester::create_session()
{
    sip.set_service_port(10010);
    sip.init("5061");
}



void SipTester::send_messages()
{
    sip.send_request("h264.1");
}
