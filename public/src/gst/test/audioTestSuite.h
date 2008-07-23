
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
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _AUDIO_TEST_SUITE_H_
#define _AUDIO_TEST_SUITE_H_

#include <cpptest.h>

#define AUDIO 1
#define RTP 0

class AudioTestSuite : public Test::Suite
{
public:

    AudioTestSuite():id_(0)
    {
#if AUDIO
        TEST_ADD(AudioTestSuite::start_1ch_audiotest)
        TEST_ADD(AudioTestSuite::stop_1ch_audiotest)
        TEST_ADD(AudioTestSuite::start_stop_1ch_audiotest)
        TEST_ADD(AudioTestSuite::start_2ch_audiotest)
        TEST_ADD(AudioTestSuite::stop_2ch_audiotest)
        TEST_ADD(AudioTestSuite::start_stop_2ch_audiotest)
        TEST_ADD(AudioTestSuite::start_6ch_audiotest);
        TEST_ADD(AudioTestSuite::stop_6ch_audiotest)
        TEST_ADD(AudioTestSuite::start_stop_6ch_audiotest)
        TEST_ADD(AudioTestSuite::start_8ch_audiotest)
        TEST_ADD(AudioTestSuite::stop_8ch_audiotest)
        TEST_ADD(AudioTestSuite::start_stop_8ch_audiotest)
        TEST_ADD(AudioTestSuite::start_8ch_comp_audiofile)
        TEST_ADD(AudioTestSuite::stop_8ch_comp_audiofile)
        TEST_ADD(AudioTestSuite::start_stop_8ch_comp_audiofile)
        TEST_ADD(AudioTestSuite::start_8ch_alsa)
        TEST_ADD(AudioTestSuite::stop_8ch_alsa)
        TEST_ADD(AudioTestSuite::start_stop_8ch_alsa)
        TEST_ADD(AudioTestSuite::start_8ch_jack)
        TEST_ADD(AudioTestSuite::stop_8ch_jack)
        TEST_ADD(AudioTestSuite::start_stop_8ch_jack)
        TEST_ADD(AudioTestSuite::start_8ch_comp_rtp_audiofile)
        TEST_ADD(AudioTestSuite::stop_8ch_comp_rtp_audiofile)
        TEST_ADD(AudioTestSuite::start_stop_8ch_comp_rtp_audiofile)
#if RTP // AND AUDIO
        TEST_ADD(AudioTestSuite::start_2ch_comp_rtp_audiotest)
        TEST_ADD(AudioTestSuite::stop_2ch_comp_rtp_audiotest)
        TEST_ADD(AudioTestSuite::start_stop_2ch_comp_rtp_audiotest)
        TEST_ADD(AudioTestSuite::start_8ch_comp_rtp_audiotest)
        TEST_ADD(AudioTestSuite::stop_8ch_comp_rtp_audiotest)
        TEST_ADD(AudioTestSuite::start_stop_8ch_comp_rtp_audiotest)

#endif                          // RTP  && AUDIO
#endif                          // AUDIO
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

    void start_2ch_comp_rtp_audiotest();
    void stop_2ch_comp_rtp_audiotest();
    void start_stop_2ch_comp_rtp_audiotest();

    void start_8ch_comp_rtp_audiotest();
    void stop_8ch_comp_rtp_audiotest();
    void start_stop_8ch_comp_rtp_audiotest();

    void start_8ch_comp_audiofile();
    void stop_8ch_comp_audiofile();
    void start_stop_8ch_comp_audiofile();

    void start_8ch_comp_rtp_audiofile();
    void stop_8ch_comp_rtp_audiofile();
    void start_stop_8ch_comp_rtp_audiofile();

    void start_8ch_jack();
    void stop_8ch_jack();
    void start_stop_8ch_jack();

    void start_8ch_alsa();
    void stop_8ch_alsa();
    void start_stop_8ch_alsa();

    void start_8ch_comp_rtp_audiofile_dv();
    void stop_8ch_comp_rtp_audiofile_dv();
    void start_stop_8ch_comp_rtp_audiofile_dv();

};


#endif // _AUDIO_TEST_SUITE_H_
