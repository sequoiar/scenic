#include <iostream>
#include "sipSingleton.h"
#include "sipTester.h"


int main(int argc, char *argv[])
{
    SipSingleton &sip = *SipSingleton::Instance();

    SipTester client(sip);
    client.create_session();
    client.send_messages();

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

