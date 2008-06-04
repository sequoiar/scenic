
// sdpTestSuite.cpp


#include <cpptest.h>
#include <iostream>

#include "sdp.h"

#include "sdpTestSuite.h"

/*----------------------------------------------*/ 
// To actually observe the tests (watch/listen), set
// this macro to 1.
/*----------------------------------------------*/ 

#define BLOCKING 1

void SdpTestSuite::setup()
{
    std::cout.flush();
    std::cout << std::endl;
}



void SdpTestSuite::tear_down()
{
    // empty
}



void SdpTestSuite::init_test()
{
    std::cout << "Init Test" << std::endl;
#if BLOCKING
    block();
#endif
}

void SdpTestSuite::sdp_header()
{
    Sdp sdp("Try","127.0.0.1");

    std::cout << sdp.str() << std::endl;

    TEST_ASSERT(!sdp.str().empty());
}


void SdpTestSuite::sdp_video()
{

    Sdp sdp("Try","127.0.0.1");
    SdpVideo sdpv("192.168.1.183",10010);

    sdp.add_media(sdpv);

    std::cout << sdp.str() << std::endl;

    TEST_ASSERT(!sdp.str().empty());
}

void SdpTestSuite::sdp_audio()
{

    Sdp sdp("Try","127.0.0.1");

    std::cout << sdp.str() << std::endl;

    TEST_ASSERT(!sdp.str().empty());
}

void SdpTestSuite::sdp_av()
{

    Sdp sdp("Try","127.0.0.1");

    std::cout << sdp.str() << std::endl;

    TEST_ASSERT(!sdp.str().empty());
}



int main(int argc, char** argv)
{
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    SdpTestSuite tester;
    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

