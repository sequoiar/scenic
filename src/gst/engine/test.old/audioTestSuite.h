
// audioTestSuite.h
// Copyright (C) 2009 Société des arts technologiques (SAT)
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
// // You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _AUDIO_TEST_SUITE_H_
#define _AUDIO_TEST_SUITE_H_

#include <cpptest.h>
#include "gstTestSuite.h"

class AudioTestSuite
    : public GstTestSuite
{
    public:

        AudioTestSuite()
        {
            testLength_ = 10000;
            
            TEST_ADD(AudioTestSuite::start_stop_6ch_pulse)

            TEST_ADD(AudioTestSuite::start_stop_8ch_alsa)
            
            TEST_ADD(AudioTestSuite::start_stop_8ch_jack)

            TEST_ADD(AudioTestSuite::start_stop_8ch_audiofile)

            TEST_ADD(AudioTestSuite::start_stop_1ch_audiotest)
            
            TEST_ADD(AudioTestSuite::start_stop_2ch_audiotest)
            
            TEST_ADD(AudioTestSuite::start_stop_audio_dv)
            
            TEST_ADD(AudioTestSuite::start_stop_6ch_audiotest)
            
            TEST_ADD(AudioTestSuite::start_stop_8ch_audiotest)
            /*----------------------------------------------*/
            /*      SANDBOX                                 */
            /*                                              */
            /*  Put newer tests here and set all defs to 0  */
            /*  to test them by themselves.                 */
            /*----------------------------------------------*/
        }

// some tests

    private:

        void start_stop_1ch_audiotest();

        void start_stop_2ch_audiotest();

        void start_stop_6ch_audiotest();

        void start_stop_8ch_audiotest();

        void start_stop_8ch_audiofile();

        void start_stop_8ch_jack();

        void start_stop_8ch_alsa();

        void start_stop_6ch_pulse();

        void start_stop_8ch_pulse();

        void start_stop_audio_dv();
};

#endif // _AUDIO_TEST_SUITE_H_

