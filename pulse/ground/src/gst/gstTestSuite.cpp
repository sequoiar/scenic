
// gstTestSuite.cpp


#include <cpptest.h>
#include <iostream>

#include "gstTestSuite.h"
#include "videoSender.h"
#include "videoReceiver.h"
#include "audioSender.h"
#include "audioReceiver.h"

/*----------------------------------------------*/ 
// To actually observe the tests (watch/listen), set
// this macro to 1.
/*----------------------------------------------*/ 

#define BLOCKING 1

int GstTestSuite::testCounter_ = 0;

void GstTestSuite::set_id(int id)
{
    if (id == 1 || id == 0)
        id_ = id;
    else 
    {
        std::cerr << "Id must be 0 or 1." << std::endl;
        exit(1);
    }
}



void GstTestSuite::setup()
{
    std::cout.flush();
    std::cout << std::endl;
    std::cout << "This is test " << testCounter_ << std::endl;
    testCounter_++;
}



void GstTestSuite::tear_down()
{
    // empty
}



void GstTestSuite::init_test()
{
    VideoSender tx;
    tx.init("test", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
}



void GstTestSuite::start_video()
{
    VideoSender tx;
    tx.init("test", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_video()
{
    VideoSender tx;
    tx.init("test", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_stop_video()
{
    VideoSender tx;

    tx.init("test", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_v4l()
{
    VideoSender tx;
    tx.init("v4l", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_v4l()
{
    VideoSender tx;
    tx.init("v4l", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_stop_v4l()
{
    VideoSender tx;

    tx.init("v4l", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_v4l_rtp()
{
    if (id_ == 0)
    {
        VideoSender tx;
        tx.init("v4lRtp", 10010, MY_ADDRESS);
        tx.start();
#if BLOCKING
        block();
#endif
    }
    else
    {
        VideoReceiver rx;
        rx.init(10010);
        rx.start();
#if BLOCKING
        block();
#endif
    }
}



void GstTestSuite::stop_v4l_rtp()
{
    VideoSender tx;
    tx.init("v4lRtp", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_stop_v4l_rtp()
{
    VideoSender tx;

    tx.init("v4lRtp", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_dv()
{
    VideoSender tx;
    tx.init("dv", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_dv()
{
    VideoSender tx;
    tx.init("dv", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_stop_dv()
{
    VideoSender tx;

    tx.init("dv", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_dv_rtp()
{
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0)
    {
        VideoReceiver rx;
        rx.init(10010);
        rx.start();
#if BLOCKING
        block();
#endif
    }
    else
    {
        VideoSender tx;
        tx.init("dvRtp", 10010, MY_ADDRESS);
        tx.start();
#if BLOCKING
        block();
#endif
    }
}



void GstTestSuite::stop_dv_rtp()
{
    VideoSender tx;
    tx.init("dvRtp", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_stop_dv_rtp()
{
    VideoSender tx;

    tx.init("dvRtp", 10010, MY_ADDRESS);
    tx.start();
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_1ch_audio()
{
    AudioSender tx;

    tx.init("1chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_1ch_audio()
{
    AudioSender tx;
    tx.init("1chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.stop());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::start_stop_1ch_audio()
{
    AudioSender tx;

    tx.init("1chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_2ch_audio()
{
    AudioSender tx;

    tx.init("2chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_2ch_audio()
{
    AudioSender tx;
    tx.init("2chTest", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_2ch_audio()
{
    AudioSender tx;

    tx.init("2chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_6ch_audio()
{
    AudioSender tx;

    tx.init("6chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_6ch_audio()
{
    AudioSender tx;
    tx.init("6chTest", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_6ch_audio()
{
    AudioSender tx;

    tx.init("6chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_8ch_audio()
{
    AudioSender tx;

    tx.init("8chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_8ch_audio()
{
    AudioSender tx;
    tx.init("8chTest", 10010, MY_ADDRESS);
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_8ch_audio()
{
    AudioSender tx;

    tx.init("8chTest", 10010, MY_ADDRESS);
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}


/*----------------------------------------------*/ 
/* Forks, a little much for a unit test.        */
/*----------------------------------------------*/ 

void GstTestSuite::start_8ch_comp_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;

    tx.init("8chCompRtpTest", 10010, MY_ADDRESS);
    rx.init(10010);
    TEST_ASSERT(tx.start());
    TEST_ASSERT(rx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_8ch_comp_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;
    tx.init("8chCompRtpTest", 10010, MY_ADDRESS);
    rx.init(10010);
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(rx.stop());
}



void GstTestSuite::start_stop_8ch_comp_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;

    tx.init("8chCompRtpTest", 10010, MY_ADDRESS);
    rx.init(10010);
    TEST_ASSERT(tx.start());
    TEST_ASSERT(rx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(rx.stop());
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_8ch_uncomp_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;

    tx.init("8chUncompRtpTest", 10010, MY_ADDRESS);
    rx.init(10010);
    TEST_ASSERT(tx.start());
    TEST_ASSERT(rx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_8ch_uncomp_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;
    tx.init("8chUncompRtpTest", 10010, MY_ADDRESS);
    rx.init(10010);
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(rx.stop());
}



void GstTestSuite::start_stop_8ch_uncomp_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;

    tx.init("8chUncompRtpTest", 10010, MY_ADDRESS);
    rx.init(10010);
    TEST_ASSERT(tx.start());
    TEST_ASSERT(rx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(rx.stop());
    TEST_ASSERT(tx.stop());
}



int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << "gstTester <0/1>" << std::endl;
        exit(1);
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    GstTestSuite tester; 
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

