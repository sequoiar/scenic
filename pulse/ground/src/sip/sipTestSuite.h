// sipTester.h

#ifndef _SIP_TEST_SUITE_H_
#define _SIP_TEST_SUITE_H_

#include <cpptest.h>

class SipTestSuite : public Test::Suite
{
    public:

        SipTestSuite()
        {
            TEST_ADD(SipTestSuite::first_test)
            TEST_ADD(SipTestSuite::second_test)
        }
        
        // some tests
#if 0
        void create_session();
        
        void create_req_session();

        void send_messages();
    
    private:
        SipSingleton &sip;
#endif

    private:
        void first_test();        
        void second_test();
};

#endif // _SIP_TESTER_H_
