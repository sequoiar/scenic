
#include <cpptest.h>
#include <iostream>

#include "gstTestSuite.h"
#include "videoSender.h"
#include "audioSender.h"
#include "audioReceiver.h"

#define BLOCKING 1

void GstTestSuite::setup()
{
    // empty
}



void GstTestSuite::tear_down()
{
    // empty
}




void GstTestSuite::block()
{
    char c;
    std::cout << __FILE__ << ": blocking, enter any key." << std::endl;
    std::cin >> c;
}



void GstTestSuite::init_test()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
#if BLOCKING
    block();
#endif
}



void GstTestSuite::start_video()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
    tx.start();
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_video()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_stop_video()
{
    VideoSender tx;

    tx.init(10010, THEIR_ADDRESS, "test");
    tx.start();
#if BLOCKING
    block();
#endif
    tx.stop();
}



void GstTestSuite::start_mono_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "monoTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_mono_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "monoTest");
    TEST_ASSERT(tx.stop());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::start_stop_mono_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "monoTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stereo_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "stereoTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_stereo_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "stereoTest");
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_stereo_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "stereoTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_multi_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "multiTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_multi_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "multiTest");
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_multi_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "multiTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_multi_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;

    tx.init(10010, THEIR_ADDRESS, "multiRtpTest");
    rx.init(10010);
    TEST_ASSERT(tx.start());
    TEST_ASSERT(rx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_multi_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;
    tx.init(10010, THEIR_ADDRESS, "multiRtpTest");
    rx.init(10010);
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(rx.stop());
}



void GstTestSuite::start_stop_multi_rtp_audio()
{
    AudioSender tx;
    AudioSender rx;

    tx.init(10010, THEIR_ADDRESS, "multiRtpTest");
    rx.init(10010, THEIR_ADDRESS, "multiRtpTest");
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
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    GstTestSuite tester;
    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

