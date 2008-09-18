
// rtpRtpAudioTestSuite.h
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
// // You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _RTP_AUDIO_TEST_SUITE_H_
#define _RTP_AUDIO_TEST_SUITE_H_

#include <cpptest.h>
#include "gstTestSuite.h"

class RtpAudioTestSuite
    : public GstTestSuite
{
    public:

        RtpAudioTestSuite()
            : fileLocation_("test_signal8.wav")
        {
            testLength_ = 10000;
            TEST_ADD(RtpAudioTestSuite::start_2ch_rtp_audiotest)
            TEST_ADD(RtpAudioTestSuite::stop_2ch_rtp_audiotest)
            TEST_ADD(RtpAudioTestSuite::start_stop_2ch_rtp_audiotest)
            TEST_ADD(RtpAudioTestSuite::start_8ch_rtp_audiotest)
            TEST_ADD(RtpAudioTestSuite::stop_8ch_rtp_audiotest)
            TEST_ADD(RtpAudioTestSuite::start_stop_8ch_rtp_audiotest)
            TEST_ADD(RtpAudioTestSuite::start_8ch_rtp_audiofile)
            TEST_ADD(RtpAudioTestSuite::stop_8ch_rtp_audiofile)
            TEST_ADD(RtpAudioTestSuite::start_stop_8ch_rtp_audiofile)
            TEST_ADD(RtpAudioTestSuite::start_audio_dv_rtp)
            TEST_ADD(RtpAudioTestSuite::stop_audio_dv_rtp)
            TEST_ADD(RtpAudioTestSuite::start_stop_audio_dv_rtp)

            /*----------------------------------------------*/
            /*      SANDBOX                                 */
            /*                                              */
            /*  Put newer tests here and set all defs to 0  */
            /*  to test them by themselves.                 */
            /*----------------------------------------------*/
        }


// some tests

    private:
        std::string fileLocation_;

        void start_2ch_rtp_audiotest();
        void stop_2ch_rtp_audiotest();
        void start_stop_2ch_rtp_audiotest();

        void start_8ch_rtp_audiotest();
        void stop_8ch_rtp_audiotest();
        void start_stop_8ch_rtp_audiotest();

        void start_8ch_rtp_audiofile();
        void stop_8ch_rtp_audiofile();
        void start_stop_8ch_rtp_audiofile();

        void start_audio_dv_rtp();
        void stop_audio_dv_rtp();
        void start_stop_audio_dv_rtp();
};


#endif // _AUDIO_TEST_SUITE_H_
