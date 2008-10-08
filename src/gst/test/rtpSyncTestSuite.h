
// synTestSuiteRtp.h
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

#ifndef _SYNC_TEST_SUITE_RTP_H_
#define _SYNC_TEST_SUITE_RTP_H_

#include "gstTestSuite.h"

#include <cpptest.h>

class SyncTestSuiteRtp
    : public GstTestSuite
{
    public:

        SyncTestSuiteRtp()
        {

            TEST_ADD(SyncTestSuiteRtp::start_dv_audio_dv_video_rtp)
            TEST_ADD(SyncTestSuiteRtp::stop_dv_audio_dv_video_rtp)
            TEST_ADD(SyncTestSuiteRtp::start_stop_dv_audio_dv_video_rtp)

            TEST_ADD(SyncTestSuiteRtp::start_8ch_comp_rtp_audiofile_dv)
            TEST_ADD(SyncTestSuiteRtp::stop_8ch_comp_rtp_audiofile_dv)
            TEST_ADD(SyncTestSuiteRtp::start_stop_8ch_comp_rtp_audiofile_dv)

        TEST_ADD(SyncTestSuiteRtp::start_audiotest_videotest_rtp);
        TEST_ADD(SyncTestSuiteRtp::stop_audiotest_videotest_rtp);
        TEST_ADD(SyncTestSuiteRtp::start_stop_audiotest_videotest_rtp);
        }

// some tests

    private:

        void start_8ch_comp_rtp_audiofile_dv();
        void stop_8ch_comp_rtp_audiofile_dv();
        void start_stop_8ch_comp_rtp_audiofile_dv();

        void start_dv_audio_dv_video_rtp();
        void stop_dv_audio_dv_video_rtp();
        void start_stop_dv_audio_dv_video_rtp();
        
        void start_audiotest_videotest_rtp();
        void stop_audiotest_videotest_rtp();
        void start_stop_audiotest_videotest_rtp();
};

#endif // _SYNC_TEST_SUITE_RTP_H_

