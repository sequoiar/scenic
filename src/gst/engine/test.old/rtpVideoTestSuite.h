
// rtpVideoTestSuite.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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
            testLength_ = 80000;

#ifdef CONFIG_GL
            TEST_ADD(RtpVideoTestSuite::start_stop_v4l_gl)
            TEST_ADD(RtpVideoTestSuite::start_stop_test_video_gl)
            TEST_ADD(RtpVideoTestSuite::start_stop_dv_gl)
#endif
            
            TEST_ADD(RtpVideoTestSuite::start_stop_mpeg4_v4l)

            TEST_ADD(RtpVideoTestSuite::start_stop_test_video)
           
            TEST_ADD(RtpVideoTestSuite::start_stop_h263_v4l)
            
            TEST_ADD(RtpVideoTestSuite::start_stop_mpeg4)
            
            TEST_ADD(RtpVideoTestSuite::start_stop_v4l)
            
            TEST_ADD(RtpVideoTestSuite::start_stop_h263)
            

            TEST_ADD(RtpVideoTestSuite::start_stop_file)

            TEST_ADD(RtpVideoTestSuite::start_stop_dv)
            

            /*----------------------------------------------*/
            /*      SANDBOX                                 */
            /*                                              */
            /*  Put newer tests here and set all defs to 0  */
            /*  to test them by themselves.                 */
            /*----------------------------------------------*/
        }


        // some tests

    private:
        void start_stop_h263();
        
        void start_stop_h263_v4l();

        void start_stop_mpeg4_v4l();

        void start_stop_mpeg4();

        void start_stop_test_video();
        
        void start_stop_test_video_gl();

        void start_stop_v4l();

        void start_stop_v4l_gl();

        void start_stop_dv();

        void start_stop_dv_gl();
    
        void start_stop_file();
};

#endif // _VIDEO_TEST_SUITE_H_

