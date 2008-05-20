// sipTester.h

#ifndef _SIP_TEST_SUITE_H_
#define _SIP_TEST_SUITE_H_

#include <cpptest.h>
#include "sipSingleton.h"

class SipTestSuite : public Test::Suite
{
    public:

        SipTestSuite()
        {
            TEST_ADD(SipTestSuite::init_test)
            TEST_ADD(SipTestSuite::instance_test)
        }
        
        // some tests
#if 0
        void create_session();
        
        void create_req_session();

        void send_messages();
    
    private:
        SipSingleton &sip;
#endif
    protected:
        virtual void setup();       // setup resources common to all tests  
        virtual void tear_down();   // destroy common resources

    private:
        SipSingleton *sip_;
        void instance_test();        
        void init_test();

};

#endif // _SIP_TESTER_H_
