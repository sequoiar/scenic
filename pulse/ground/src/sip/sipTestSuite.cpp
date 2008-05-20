
#include <cpptest.h>
#include "sipTestSuite.h"
#include "sipSingleton.h"
#include "../gst/defaultAddresses.h"

void SipTestSuite::setup()
{
    sip_ = SipSingleton::Instance();
}




void SipTestSuite::tear_down()
{
    // empty
}



void SipTestSuite::init_test()
{
    sip_->init(MY_ADDRESS, "5060", THEIR_ADDRESS, "5061");
//    TEST_ASSERT(sip_ != 0);
}



void SipTestSuite::instance_test()
{
    TEST_ASSERT(SipSingleton::Instance() != 0);
}



int main(int argc, char** argv)
{
    SipTestSuite sts;
    Test::TextOutput output(Test::TextOutput::Verbose);
    return sts.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#if 0
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

#endif 
