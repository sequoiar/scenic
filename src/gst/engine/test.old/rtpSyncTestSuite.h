
// synTestSuiteRtp.h
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

#ifndef _RTP_SYNC_TEST_SUITE_H_
#define _RTP_SYNC_TEST_SUITE_H_

#include "gstTestSuite.h"

#include <cpptest.h>

class SyncTestSuiteRtp
: public GstTestSuite
{
    public:

        SyncTestSuiteRtp()
        {
            // DANGER: if this is too short you're likely to have race cond's.
            testLength_ = 10000000;
            TEST_ADD(SyncTestSuiteRtp::start_stop_jack_v4l)


            TEST_ADD(SyncTestSuiteRtp::start_stop_dv_audio_dv_video)

            TEST_ADD(SyncTestSuiteRtp::start_stop_jack_v4l_vorbis)

            TEST_ADD(SyncTestSuiteRtp::start_stop_audiotest_videotest)

            TEST_ADD(SyncTestSuiteRtp::start_stop_8ch_audiofile_dv)


        }

        // some tests

    private:

        void start_stop_8ch_audiofile_dv();

        void start_stop_dv_audio_dv_video();

        void start_stop_audiotest_videotest();

        void start_stop_jack_v4l();

        void start_stop_jack_v4l_vorbis();
};

#endif // _RTP_SYNC_TEST_SUITE_H_

