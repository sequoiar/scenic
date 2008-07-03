
// gstTestSuite.h

#ifndef _SDP_TEST_SUITE_H_
#define _SDP_TEST_SUITE_H_

#include <cpptest.h>

class SdpTestSuite : public Test::Suite
{
public:

	SdpTestSuite()
	{
		TEST_ADD(SdpTestSuite::init_test)
		TEST_ADD(SdpTestSuite::sdp_header)
		TEST_ADD(SdpTestSuite::sdp_parse)
		TEST_ADD(SdpTestSuite::sdp_video)
		TEST_ADD(SdpTestSuite::sdp_audio) TEST_ADD(SdpTestSuite::sdp_av)
	}

// some tests

protected:
	virtual void setup();       // setup resources common to all tests
	virtual void tear_down();   // destroy common resources

private:
	void init_test();
	void sdp_header();
	void sdp_parse();
	void sdp_video();
	void sdp_audio();
	void sdp_av();
};

#define BLOCKING 1

#if BLOCKING
#define BLOCK() std::cout.flush();                              \
    std::cout << __FILE__ << ":" << __LINE__        \
              << ": blocking, enter any key." << std::endl;   \
    std::cin.get()
#elif
#define BLOCK()
#endif

#endif // _SDP_TEST_SUITE_H_
