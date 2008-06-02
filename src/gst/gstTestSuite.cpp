
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



void GstTestSuite::start_1ch_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "1chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_1ch_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "1chTest");
    TEST_ASSERT(tx.stop());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::start_stop_1ch_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "1chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_2ch_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "2chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_2ch_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "2chTest");
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_2ch_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "2chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_6ch_audio()
{
    std::cout << "I tend to hang, possibly related to the errormsg I put out?" << std::endl;
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "6chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_6ch_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "6chTest");
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_6ch_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "6chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_8ch_audio()
{
    std::cout << "I tend to hang, possibly related to the errormsg I put out?" << std::endl;
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "8chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
}



void GstTestSuite::stop_8ch_audio()
{
    AudioSender tx;
    tx.init(10010, THEIR_ADDRESS, "8chTest");
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_stop_8ch_audio()
{
    AudioSender tx;

    tx.init(10010, THEIR_ADDRESS, "8chTest");
    TEST_ASSERT(tx.start());
#if BLOCKING
    block();
#endif
    TEST_ASSERT(tx.stop());
}



void GstTestSuite::start_8ch_comp_rtp_audio()
{
    AudioSender tx;
    AudioReceiver rx;

    tx.init(10010, THEIR_ADDRESS, "8chRtpTest");
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
    tx.init(10010, THEIR_ADDRESS, "8chRtpTest");
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
    AudioSender rx;

    tx.init(10010, THEIR_ADDRESS, "8chCompRtpTest");
    rx.init(10010, THEIR_ADDRESS, "8chCompRtpTest");
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

    tx.init(10010, THEIR_ADDRESS, "8chUncompRtpTest");
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
    tx.init(10010, THEIR_ADDRESS, "8chUncompRtpTest");
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
    AudioSender rx;

    tx.init(10010, THEIR_ADDRESS, "8chUncompRtpTest");
    rx.init(10010, THEIR_ADDRESS, "8chUncompRtpTest");
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

