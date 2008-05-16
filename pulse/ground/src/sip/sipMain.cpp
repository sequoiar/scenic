#include <iostream>
#include "sipSingleton.h"


int main(int argc, char *argv[])
{
    SipSingleton &sip = *SipSingleton::Instance();

    sip.set_service_port(10010);

    if(!sip.init(argc,argv))
        return -1;

    if (argc == 5)
    {
        sip.send_request("h264.1");
    }


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

