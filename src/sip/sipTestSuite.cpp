
// sipTestSuite.cpp

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
}



void SipTestSuite::instance_test()
{
    TEST_ASSERT(SipSingleton::Instance() != 0);
}



void SipTestSuite::create_session()
{
    sip_->set_service_port(10010);
    //sip_->init("5061");
}



void SipTestSuite::send_messages()
{
    sip_->send_request("DV");
}



int main(int argc, char** argv)
{
    SipTestSuite sts;
    Test::TextOutput output(Test::TextOutput::Verbose);
    return sts.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

