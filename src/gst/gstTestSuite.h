
// gstTestSuite.h
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

// Declares and registers a series of unit tests. Note that currently, these tests are quite simple. 
// A more comprehensive/detailed testsuite would have to cover specific
// usecases and look more like client code. The current set of tests just does a quick probe of our 
// media interface.

#ifndef _GST_TEST_SUITE_H_
#define _GST_TEST_SUITE_H_

#include <cpptest.h>

#define BLOCKING 1
#define VIDEO 0
#define AUDIO 1
#define RTP 1


class GstTestSuite : public Test::Suite
{
    public:

        GstTestSuite()
        {
#if VIDEO
            TEST_ADD(GstTestSuite::init_test)

            TEST_ADD(GstTestSuite::start_video)
            TEST_ADD(GstTestSuite::stop_video)
            TEST_ADD(GstTestSuite::start_stop_video)

            TEST_ADD(GstTestSuite::start_dv)
            TEST_ADD(GstTestSuite::stop_dv)
            TEST_ADD(GstTestSuite::start_stop_dv)

            TEST_ADD(GstTestSuite::start_v4l)
            TEST_ADD(GstTestSuite::stop_v4l)
            TEST_ADD(GstTestSuite::start_stop_v4l)

#if RTP
            TEST_ADD(GstTestSuite::start_v4l_rtp)
            TEST_ADD(GstTestSuite::stop_v4l_rtp)
            TEST_ADD(GstTestSuite::start_stop_v4l_rtp)

            TEST_ADD(GstTestSuite::start_dv_rtp)
            TEST_ADD(GstTestSuite::stop_dv_rtp)
            TEST_ADD(GstTestSuite::start_stop_dv_rtp)
#endif // RTP
#endif // VIDEO
#if AUDIO
            TEST_ADD(GstTestSuite::start_1ch_audio)
            TEST_ADD(GstTestSuite::stop_1ch_audio)
            TEST_ADD(GstTestSuite::start_stop_1ch_audio)
            TEST_ADD(GstTestSuite::start_2ch_audio)
            TEST_ADD(GstTestSuite::stop_2ch_audio)
            TEST_ADD(GstTestSuite::start_stop_2ch_audio)

            TEST_ADD(GstTestSuite::start_6ch_audio);
            TEST_ADD(GstTestSuite::stop_6ch_audio)
            TEST_ADD(GstTestSuite::start_stop_6ch_audio)

            TEST_ADD(GstTestSuite::start_8ch_audio)
            TEST_ADD(GstTestSuite::stop_8ch_audio)
            TEST_ADD(GstTestSuite::start_stop_8ch_audio)
#if RTP
            TEST_ADD(GstTestSuite::start_2ch_comp_rtp_audio)
            TEST_ADD(GstTestSuite::stop_2ch_comp_rtp_audio)
            TEST_ADD(GstTestSuite::start_stop_2ch_comp_rtp_audio)

            TEST_ADD(GstTestSuite::start_8ch_comp_rtp_audio)
            TEST_ADD(GstTestSuite::stop_8ch_comp_rtp_audio)
            TEST_ADD(GstTestSuite::start_stop_8ch_comp_rtp_audio)

#endif // RTP
#endif // AUDIO

#if 0
                TEST_ADD(GstTestSuite::start_8ch_uncomp_rtp_audio)
                TEST_ADD(GstTestSuite::stop_8ch_uncomp_rtp_audio)
                TEST_ADD(GstTestSuite::start_stop_8ch_uncomp_rtp_audio)
#endif
        }

        void set_id(int id);

        // some tests

    protected:
        virtual void setup();       // setup resources common to all tests  
        virtual void tear_down();   // destroy common resources

    private:
        int id_;
        void block(); // inline
        void init_test();

        void start_video();
        void stop_video();
        void start_stop_video();

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

        void start_1ch_audio();
        void stop_1ch_audio();
        void start_stop_1ch_audio();

        void start_2ch_audio();
        void stop_2ch_audio();
        void start_stop_2ch_audio();

        void start_6ch_audio();
        void stop_6ch_audio();
        void start_stop_6ch_audio();

        void start_8ch_audio();
        void stop_8ch_audio();
        void start_stop_8ch_audio();

        void start_2ch_comp_rtp_audio();
        void stop_2ch_comp_rtp_audio();
        void start_stop_2ch_comp_rtp_audio();

        void start_8ch_comp_rtp_audio();
        void stop_8ch_comp_rtp_audio();
        void start_stop_8ch_comp_rtp_audio();

        void start_8ch_uncomp_rtp_audio();
        void stop_8ch_uncomp_rtp_audio();
        void start_stop_8ch_uncomp_rtp_audio();
};

#if BLOCKING
    #define BLOCK() std::cout.flush();                              \
                    std::cout << __FILE__ << ":" << __LINE__        \
                    << ": blocking, enter any key." << std::endl;   \
                    std::cin.get()
#elif
    #define BLOCK()
#endif 


#endif // _GST_TEST_SUITE_H_

