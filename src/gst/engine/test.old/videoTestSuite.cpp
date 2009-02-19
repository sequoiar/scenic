/* videoTestSuite.cpp
 * Copyright (C) 2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include <cpptest.h>
#include <cstdlib>
#include "videoTestSuite.h"
#include "videoLocal.h"
#include "videoConfig.h"
#include "videoSink.h"
#include "playback.h"

const int VideoTestSuite::GL_SCREEN = 0;


void VideoTestSuite::start_stop_test_video_gl()
{
    VideoSourceConfig srcConfig("videotestsrc", 3000000);
    VideoSinkConfig sinkConfig("glimagesink", GL_SCREEN);
    VideoLocal tx(srcConfig, sinkConfig);
    tx.init();

    playback::start();
    

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}


void VideoTestSuite::start_stop_test_video()
{
    VideoSourceConfig srcConfig("videotestsrc", 3000000);
    VideoSinkConfig sinkConfig("xvimagesink");
    VideoLocal tx(srcConfig, sinkConfig);
    tx.init();

    playback::start();
    

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void VideoTestSuite::start_stop_v4l()
{
    VideoSourceConfig srcConfig("v4l2src", 3000000);
    VideoSinkConfig sinkConfig("xvimagesink");
    VideoLocal tx(srcConfig, sinkConfig);
    tx.init();

    playback::start();
    

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void VideoTestSuite::start_stop_v4l_gl()
{
    VideoSourceConfig srcConfig("v4l2src", 3000000);
    VideoSinkConfig sinkConfig("glimagesink");
    VideoLocal tx(srcConfig, sinkConfig);
    tx.init();

    playback::start();
    

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}


void VideoTestSuite::start_stop_dv()
{
    VideoSourceConfig srcConfig("dv1394src", 3000000);
    VideoSinkConfig sinkConfig("xvimagesink");
    VideoLocal tx(srcConfig, sinkConfig);
    tx.init();

    playback::start();
    

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void VideoTestSuite::start_stop_file()
{
    VideoSourceConfig srcConfig("filesrc", 3000000, videoFilename_);
    VideoSinkConfig sinkConfig("xvimagesink");
    VideoLocal tx(srcConfig, sinkConfig);
    tx.init();

    playback::start();
    
    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}


int mainVideoTestSuite(int /*argc*/, char ** /*argv*/)
{
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    VideoTestSuite tester;

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

