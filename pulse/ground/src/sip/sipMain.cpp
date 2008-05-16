#include <iostream>
#include "sipSingleton.h"
#include "sipTester.h"


int main(int argc, char *argv[])
{
    SipSingleton &sip = *SipSingleton::Instance();

    SipTester client(sip);

    if (argc > 1)           //  any arg will do
    {
        client.create_req_session();
        client.send_messages();
    }
    else
        client.create_session();


    for (;;)
    {
        static int eventCount;
        if (eventCount += sip.handle_events())
        {
            std::cout << "HANDLED " << eventCount << " EVENTS " << std::endl;
        }
    }

    return 0;
}

