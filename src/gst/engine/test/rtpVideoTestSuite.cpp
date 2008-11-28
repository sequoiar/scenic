
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
#include <cassert>
#include <iostream>
#include <cstdlib>
#include "rtpVideoTestSuite.h"
#include "videoSender.h"
#include "videoConfig.h"
#include "videoReceiver.h"
#include "playback.h"
#include "hostIP.h"
#include "tcp/tcpThread.h"
#include "tcp/parser.h"


static std::auto_ptr<VideoReceiver> buildVideoReceiver(const char *videoSink = "xvimagesink")
{
        VideoSinkConfig vConfig(videoSink);
        ReceiverConfig rConfig("h264", get_host_ip(), GstTestSuite::V_PORT, "");
        std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
        rx->init();
        return rx;
}


static std::auto_ptr<VideoSender> buildVideoSender(const VideoSourceConfig vConfig)
{
        SenderConfig rConfig("h264", get_host_ip(), GstTestSuite::V_PORT);
        std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
        tx->init();
        return tx;
}


void RtpVideoTestSuite::start_test_video()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("videotestsrc");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void RtpVideoTestSuite::stop_test_video()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("videotestsrc");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_test_video()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("videotestsrc");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_v4l()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void RtpVideoTestSuite::stop_v4l()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_v4l()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}



void RtpVideoTestSuite::start_v4l_gl()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver("glimagesink"));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void RtpVideoTestSuite::stop_v4l_gl()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver("glimagesink"));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_v4l_gl()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver("glimagesink"));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_dv()
{
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void RtpVideoTestSuite::stop_dv()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_dv()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_dv_gl()
{
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver("glimagesink"));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void RtpVideoTestSuite::stop_dv_gl()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver("glimagesink"));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_dv_gl()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver("glimagesink"));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}

void RtpVideoTestSuite::start_file()
{
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("filesrc", videoFilename_);
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void RtpVideoTestSuite::stop_file()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("filesrc", videoFilename_);
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        BLOCK();

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpVideoTestSuite::start_stop_file()
{
    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> rx(buildVideoReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("filesrc", videoFilename_);
        
        std::auto_ptr<VideoSender> tx(buildVideoSender(vConfig));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


int mainRtpVideoTestSuite(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "rtpVideoTester <0/1>" << std::endl;
        return 1;
    }
    
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    RtpVideoTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    try {
        return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    catch (Except e)
    {
        std::cerr << e.msg_;
        return 1;
    }
}


