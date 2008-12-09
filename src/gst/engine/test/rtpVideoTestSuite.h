
// rtpVideoTestSuite.h
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

class RtpVideoTestSuite
    : public GstTestSuite
{
    public:

        RtpVideoTestSuite()
        {
            testLength_ = 800000;
            
            TEST_ADD(RtpVideoTestSuite::start_mpeg4_v4l)
            TEST_ADD(RtpVideoTestSuite::stop_mpeg4_v4l)
            TEST_ADD(RtpVideoTestSuite::start_stop_mpeg4_v4l)
            
            TEST_ADD(RtpVideoTestSuite::start_mpeg4)
            TEST_ADD(RtpVideoTestSuite::stop_mpeg4)
            TEST_ADD(RtpVideoTestSuite::start_stop_mpeg4)
            
            TEST_ADD(RtpVideoTestSuite::start_test_video)
            TEST_ADD(RtpVideoTestSuite::stop_test_video)
            TEST_ADD(RtpVideoTestSuite::start_stop_test_video)

            TEST_ADD(RtpVideoTestSuite::start_v4l_gl)
            TEST_ADD(RtpVideoTestSuite::stop_v4l_gl)
            TEST_ADD(RtpVideoTestSuite::start_stop_v4l_gl)
            
            TEST_ADD(RtpVideoTestSuite::start_dv_gl)
            TEST_ADD(RtpVideoTestSuite::stop_dv_gl)
            TEST_ADD(RtpVideoTestSuite::start_stop_dv_gl)

            TEST_ADD(RtpVideoTestSuite::start_dv)
            TEST_ADD(RtpVideoTestSuite::stop_dv)
            TEST_ADD(RtpVideoTestSuite::start_stop_dv)

            
            TEST_ADD(RtpVideoTestSuite::start_v4l)
            TEST_ADD(RtpVideoTestSuite::stop_v4l)
            TEST_ADD(RtpVideoTestSuite::start_stop_v4l)

            TEST_ADD(RtpVideoTestSuite::start_file)
            TEST_ADD(RtpVideoTestSuite::stop_file)
            TEST_ADD(RtpVideoTestSuite::start_stop_file)
            /*----------------------------------------------*/
            /*      SANDBOX                                 */
            /*                                              */
            /*  Put newer tests here and set all defs to 0  */
            /*  to test them by themselves.                 */
            /*----------------------------------------------*/
        }


        // some tests

    private:
        void start_mpeg4_v4l();
        void stop_mpeg4_v4l();
        void start_stop_mpeg4_v4l();

        void start_mpeg4();
        void stop_mpeg4();
        void start_stop_mpeg4();

        void start_test_video();
        void stop_test_video();
        void start_stop_test_video();

        void start_v4l();
        void stop_v4l();
        void start_stop_v4l();

        void start_v4l_gl();
        void stop_v4l_gl();
        void start_stop_v4l_gl();

        void start_dv();
        void stop_dv();
        void start_stop_dv();

        void start_dv_gl();
        void stop_dv_gl();
        void start_stop_dv_gl();
    
        void start_file();
        void stop_file();
        void start_stop_file();
};

#endif // _VIDEO_TEST_SUITE_H_

