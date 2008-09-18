
// videoTestSuite.cpp
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
#include "videoTestSuite.h"
#include "videoLocal.h"
#include "videoConfig.h"


void VideoTestSuite::start_test_video()
{
    if (id_ == 1)
        return;
    VideoConfig config("videotestsrc");
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());
}


void VideoTestSuite::stop_test_video()
{
    if (id_ == 1)
        return;
    VideoConfig config("videotestsrc");
    VideoLocal tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void VideoTestSuite::start_stop_test_video()
{
    if (id_ == 1)
        return;
    VideoConfig config("videotestsrc");
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void VideoTestSuite::start_v4l()
{
    if (id_ == 1)
        return;
    VideoConfig config("v4l2src");
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void VideoTestSuite::stop_v4l()
{
    if (id_ == 1)
        return;
    VideoConfig config("v4l2src");
    VideoLocal tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void VideoTestSuite::start_stop_v4l()
{
    if (id_ == 1)
        return;
    VideoConfig config("v4l2src");
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void VideoTestSuite::start_dv()
{
    if (id_ == 1)
        return;
    VideoConfig config("dv1394src");
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void VideoTestSuite::stop_dv()
{
    if (id_ == 1)
        return;
    VideoConfig config("dv1394src");
    VideoLocal tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void VideoTestSuite::start_stop_dv()
{
    if (id_ == 1)
        return;
    VideoConfig config("dv1394src");
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void VideoTestSuite::start_file()
{
    if (id_ == 1)
        return;
    VideoConfig config("filesrc", fileLocation_);
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void VideoTestSuite::stop_file()
{
    if (id_ == 1)
        return;
    VideoConfig config("filesrc", fileLocation_);
    VideoLocal tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void VideoTestSuite::start_stop_file()
{
    if (id_ == 1)
        return;
    VideoConfig config("filesrc", fileLocation_);
    VideoLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


int main(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "videoTester <0/1>" << std::endl;
        return 1;
    }
    
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    VideoTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

