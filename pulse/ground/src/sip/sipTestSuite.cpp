
#include <cpptest.h>
#include "sipTestSuite.h"

void SipTestSuite::first_test()
{
    TEST_ASSERT(1 + 1 == 2);

}



void SipTestSuite::second_test()
{
    TEST_ASSERT_DELTA(0.5, 0.7, 0.3);
}



int main()
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
