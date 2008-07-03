
// sdpTestSuite.cpp

#include <cpptest.h>
#include <iostream>

#include "sdp.h"

#include "sdpTestSuite.h"

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
	BLOCK();
}

void SdpTestSuite::sdp_header()
{
	Sdp sdp("Try");

	std::cout << sdp.str() << std::endl;

	TEST_ASSERT(!sdp.str().empty());
}

void SdpTestSuite::sdp_parse()
{
	Sdp sdp2;

	Sdp sdp("Try");
	SdpMedia sdpv = SdpMediaFactory::clone("H264");
	sdpv.set_ip("192.168.1.183");
	sdpv.set_port(10010);
	TEST_ASSERT(sdp.add_media(sdpv));

	sdp2.parse(sdp.str());
	std::cout << "PARSE:" << std::endl;
	std::cout << sdp2.str() << std::endl;
	TEST_ASSERT(!sdp2.str().empty());
}

void SdpTestSuite::sdp_video()
{

	Sdp sdp("Try");
	SdpMedia sdpv = SdpMediaFactory::clone("H264");
	sdpv.set_ip("192.168.1.183");
	sdpv.set_port(10010);

	TEST_ASSERT(sdp.add_media(sdpv));

	std::cout << sdp.str() << std::endl;

	TEST_ASSERT(!sdp.str().empty());

	sdp.list_media();

	BLOCK();
}

void SdpTestSuite::sdp_audio()
{
	Sdp sdp("Try");

	std::cout << sdp.str() << std::endl;

	TEST_ASSERT(!sdp.str().empty());
}

void SdpTestSuite::sdp_av()
{
	Sdp sdp("Try");

	std::cout << sdp.str() << std::endl;

	TEST_ASSERT(!sdp.str().empty());
}

int main(int argc, char **argv)
{
	std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
	SdpTestSuite tester;
	Test::TextOutput output(Test::TextOutput::Verbose);
	return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}
