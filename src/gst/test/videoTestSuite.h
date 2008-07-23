
// videoTestSuite.h
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

#ifndef _VIDEO_TEST_SUITE_H_
#define _VIDEO_TEST_SUITE_H_

#include <cpptest.h>

#define VIDEO 1
#define RTP 0

class VideoTestSuite : public Test::Suite
{
public:

    VideoTestSuite()
    {
#if VIDEO
        TEST_ADD(VideoTestSuite::init_test)
        TEST_ADD(VideoTestSuite::start_test_video)
        TEST_ADD(VideoTestSuite::stop_test_video)
        TEST_ADD(VideoTestSuite::start_stop_test_video)
        TEST_ADD(VideoTestSuite::start_dv)
        TEST_ADD(VideoTestSuite::stop_dv)
        TEST_ADD(VideoTestSuite::start_stop_dv)
        TEST_ADD(VideoTestSuite::start_v4l)
        TEST_ADD(VideoTestSuite::stop_v4l)
        TEST_ADD(VideoTestSuite::start_stop_v4l)
#if RTP // AND VIDEO
        TEST_ADD(VideoTestSuite::start_dv_rtp)
        TEST_ADD(VideoTestSuite::stop_dv_rtp)
        TEST_ADD(VideoTestSuite::start_stop_dv_rtp)
        TEST_ADD(VideoTestSuite::start_v4l_rtp)
        TEST_ADD(VideoTestSuite::stop_v4l_rtp)
        TEST_ADD(VideoTestSuite::start_stop_v4l_rtp)
#endif // RTP && VIDEO
#endif // VIDEO
        
        /*----------------------------------------------*/
        /*      SANDBOX                                 */
        /*                                              */
        /*  Put newer tests here and set all defs to 0  */
        /*  to test them by themselves.                 */
        /*----------------------------------------------*/
    }

    void set_id(int id);

// some tests

protected:
    virtual void setup();           // setup resources common to all tests
    virtual void tear_down();       // destroy common resources

private:
    int id_;

    void init_test();

    void start_test_video();
    void stop_test_video();
    void start_stop_test_video();

    void start_v4l();
    void stop_v4l();
    void start_stop_v4l();

    void start_v4l_rtp();
    void stop_v4l_rtp();
    void start_stop_v4l_rtp();

    void start_dv();
    void stop_dv();
    void start_stop_dv();

    void start_dv_rtp();
    void stop_dv_rtp();
    void start_stop_dv_rtp();
};

#endif // _VIDEO_TEST_SUITE_H_
