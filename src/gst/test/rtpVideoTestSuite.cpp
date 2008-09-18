
// rtpVideoTestSuite.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#include <cpptest.h>
#include <iostream>
#include <cstdlib>
#include "rtpVideoTestSuite.h"
#include "videoSender.h"
#include "videoConfig.h"
#include "videoReceiver.h"
#include "hostIP.h"


void RtpVideoTestSuite::start_test_video_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        VideoConfig vConfig("videotestsrc");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpVideoTestSuite::stop_test_video_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("videotestsrc");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_test_video_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("videotestsrc");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpVideoTestSuite::start_v4l_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        VideoConfig vConfig("v4l2src");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpVideoTestSuite::stop_v4l_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("v4l2src");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_v4l_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("v4l2src");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void RtpVideoTestSuite::start_dv_rtp()
{
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        VideoConfig vConfig("dv1394src");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpVideoTestSuite::stop_dv_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("dv1394src");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_dv_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("dv1394src");
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void RtpVideoTestSuite::start_file_rtp()
{
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        VideoConfig vConfig("filesrc", fileLocation_);
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpVideoTestSuite::stop_file_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("filesrc", fileLocation_);
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_file_rtp()
{
    if (id_ == 0) {
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver rx(vConfig, rConfig);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig vConfig("filesrc", fileLocation_);
        SenderConfig rConfig("h264", get_host_ip(), V_PORT);
        VideoSender tx(vConfig, rConfig);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


int main(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "rtpVideoTester <0/1>" << std::endl;
        return 1;
    }
    
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    RtpVideoTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


