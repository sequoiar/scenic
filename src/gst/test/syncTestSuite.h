
// synTestSuite.h
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

#ifndef _SYNC_TEST_SUITE_H_
#define _SYNC_TEST_SUITE_H_

#include "gstTestSuite.h"

#include <cpptest.h>

#define DV 1

class SyncTestSuite
    : public GstTestSuite
{
    public:

        SyncTestSuite()
        {
#if 0
            TEST_ADD(SyncTestSuite::start_8ch_comp_rtp_audiofile_dv)
            TEST_ADD(SyncTestSuite::stop_8ch_comp_rtp_audiofile_dv)
            TEST_ADD(SyncTestSuite::start_stop_8ch_comp_rtp_audiofile_dv)
            TEST_ADD(SyncTestSuite::start_dv_audio_dv_video)
            TEST_ADD(SyncTestSuite::stop_dv_audio_dv_video)
            TEST_ADD(SyncTestSuite::start_stop_dv_audio_dv_video)
#endif

#if DV
            TEST_ADD(SyncTestSuite::start_dv_audio_dv_video_rtp)
            TEST_ADD(SyncTestSuite::stop_dv_audio_dv_video_rtp)
            TEST_ADD(SyncTestSuite::start_stop_dv_audio_dv_video_rtp)
#endif  // DV

        }


// some tests

    private:

        void start_8ch_comp_rtp_audiofile_dv();
        void stop_8ch_comp_rtp_audiofile_dv();
        void start_stop_8ch_comp_rtp_audiofile_dv();

        void start_dv_audio_dv_video();
        void stop_dv_audio_dv_video();
        void start_stop_dv_audio_dv_video();

        void start_dv_audio_dv_video_rtp();
        void stop_dv_audio_dv_video_rtp();
        void start_stop_dv_audio_dv_video_rtp();
};

#endif // _SYNC_TEST_SUITE_H_

