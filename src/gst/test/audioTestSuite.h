
// audioTestSuite.h
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
            testLength_ = 500000;
            TEST_ADD(AudioTestSuite::start_8ch_audiofile)
            TEST_ADD(AudioTestSuite::stop_8ch_audiofile)
            TEST_ADD(AudioTestSuite::start_stop_8ch_audiofile)

            TEST_ADD(AudioTestSuite::start_8ch_jack)
            TEST_ADD(AudioTestSuite::stop_8ch_jack)
            TEST_ADD(AudioTestSuite::start_stop_8ch_jack)

            TEST_ADD(AudioTestSuite::start_1ch_audiotest)
            TEST_ADD(AudioTestSuite::stop_1ch_audiotest)
            TEST_ADD(AudioTestSuite::start_stop_1ch_audiotest)
            TEST_ADD(AudioTestSuite::start_2ch_audiotest)
            TEST_ADD(AudioTestSuite::stop_2ch_audiotest)
            TEST_ADD(AudioTestSuite::start_stop_2ch_audiotest)
            TEST_ADD(AudioTestSuite::start_audio_dv)
            TEST_ADD(AudioTestSuite::stop_audio_dv)
            TEST_ADD(AudioTestSuite::start_stop_audio_dv)
            TEST_ADD(AudioTestSuite::start_6ch_audiotest);
            TEST_ADD(AudioTestSuite::stop_6ch_audiotest)
            TEST_ADD(AudioTestSuite::start_stop_6ch_audiotest)
            TEST_ADD(AudioTestSuite::start_8ch_audiotest)
            TEST_ADD(AudioTestSuite::stop_8ch_audiotest)
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

        void start_1ch_audiotest();
        void stop_1ch_audiotest();
        void start_stop_1ch_audiotest();

        void start_2ch_audiotest();
        void stop_2ch_audiotest();
        void start_stop_2ch_audiotest();

        void start_6ch_audiotest();
        void stop_6ch_audiotest();
        void start_stop_6ch_audiotest();

        void start_8ch_audiotest();
        void stop_8ch_audiotest();
        void start_stop_8ch_audiotest();

        void start_8ch_audiofile();
        void stop_8ch_audiofile();
        void start_stop_8ch_audiofile();

        void start_8ch_jack();
        void stop_8ch_jack();
        void start_stop_8ch_jack();

        void start_audio_dv();
        void stop_audio_dv();
        void start_stop_audio_dv();
};

#endif // _AUDIO_TEST_SUITE_H_

