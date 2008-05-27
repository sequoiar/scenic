
#include <cpptest.h>
#include "gstTestSuite.h"
#include "videoSender.h"

void GstTestSuite::setup()
{
    gst_init(0, NULL);
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
}



void GstTestSuite::start_video()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
    tx.start();
}



void GstTestSuite::stop_video()
{
    VideoSender tx;
    tx.init(10010, THEIR_ADDRESS, "test");
    tx.stop();
}



int main(int argc, char** argv)
{
    GstTestSuite tester;
    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

