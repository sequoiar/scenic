
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
#include "gstTestSuite.h"

#define LOCAL 0
#define RTP 0

class VideoTestSuite
    : public GstTestSuite
{
    public:

        VideoTestSuite()
            : fileLocation_("ubuntu.ogg")
        {
            testLength_ = 5000;
#if LOCAL
            TEST_ADD(VideoTestSuite::start_test_video)
            TEST_ADD(VideoTestSuite::stop_test_video)
            TEST_ADD(VideoTestSuite::start_stop_test_video)
            TEST_ADD(VideoTestSuite::start_dv)
            TEST_ADD(VideoTestSuite::start_stop_dv)
            TEST_ADD(VideoTestSuite::start_v4l)
            TEST_ADD(VideoTestSuite::stop_v4l)
            TEST_ADD(VideoTestSuite::start_stop_v4l)
            TEST_ADD(VideoTestSuite::start_file)
            TEST_ADD(VideoTestSuite::stop_file)
            TEST_ADD(VideoTestSuite::start_stop_file)
#endif  // LOCAL
#if RTP
            TEST_ADD(VideoTestSuite::start_v4l_rtp)
            TEST_ADD(VideoTestSuite::stop_v4l_rtp)
            TEST_ADD(VideoTestSuite::start_stop_v4l_rtp)
            TEST_ADD(VideoTestSuite::start_dv_rtp)
            TEST_ADD(VideoTestSuite::stop_dv_rtp)
            TEST_ADD(VideoTestSuite::start_stop_dv_rtp)
#endif  // RTP

            /*----------------------------------------------*/
            /*      SANDBOX                                 */
            /*                                              */
            /*  Put newer tests here and set all defs to 0  */
            /*  to test them by themselves.                 */
            /*----------------------------------------------*/
            TEST_ADD(VideoTestSuite::start_file_rtp)
            TEST_ADD(VideoTestSuite::stop_file_rtp)
            TEST_ADD(VideoTestSuite::start_stop_file_rtp)
        }


        // some tests

    private:
        std::string fileLocation_;

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

        void start_file();
        void stop_file();
        void start_stop_file();

        void start_file_rtp();
        void stop_file_rtp();
        void start_stop_file_rtp();
};

#endif // _VIDEO_TEST_SUITE_H_

