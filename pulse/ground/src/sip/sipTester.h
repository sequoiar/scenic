// sipTester.h

#ifndef _SIP_TESTER_H_
#define _SIP_TESTER_H_

#include "sipSingleton.h"

class SipTester
{
    public:

        SipTester(SipSingleton &sip);
        ~SipTester();
        
        // some tests
        void create_session();

        void send_messages();
    
    private:
        SipSingleton &sip;
};

#endif // _SIP_TESTER_H_
