
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
// gstreamer interface.
// Several of the tests (specifically those sending over udp) require two instances of the Tester
// to be running, one with an argument of 0 and the other with an argument of 1.

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
		TEST_ADD(GstTestSuite::start_test_video)
		TEST_ADD(GstTestSuite::stop_test_video)
		TEST_ADD(GstTestSuite::start_stop_test_video)
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
#endif // RTP && VIDEO
#endif // VIDEO
#if AUDIO
		TEST_ADD(GstTestSuite::start_1ch_audiotest)
		TEST_ADD(GstTestSuite::stop_1ch_audiotest)
		TEST_ADD(GstTestSuite::start_stop_1ch_audiotest)
		TEST_ADD(GstTestSuite::start_2ch_audiotest)
		TEST_ADD(GstTestSuite::stop_2ch_audiotest)
		TEST_ADD(GstTestSuite::start_stop_2ch_audiotest) TEST_ADD(GstTestSuite::start_6ch_audiotest);
		TEST_ADD(GstTestSuite::stop_6ch_audiotest)
		TEST_ADD(GstTestSuite::start_stop_6ch_audiotest)
		TEST_ADD(GstTestSuite::start_8ch_audiotest)
		TEST_ADD(GstTestSuite::stop_8ch_audiotest)
		TEST_ADD(GstTestSuite::start_stop_8ch_audiotest)
		TEST_ADD(GstTestSuite::start_8ch_comp_audiofile)
		TEST_ADD(GstTestSuite::stop_8ch_comp_audiofile)
		TEST_ADD(GstTestSuite::start_stop_8ch_comp_audiofile)
#if RTP
		TEST_ADD(GstTestSuite::start_2ch_comp_rtp_audiotest)
		TEST_ADD(GstTestSuite::stop_2ch_comp_rtp_audiotest)
		TEST_ADD(GstTestSuite::start_stop_2ch_comp_rtp_audiotest)
		TEST_ADD(GstTestSuite::start_8ch_comp_rtp_audiotest)
		TEST_ADD(GstTestSuite::stop_8ch_comp_rtp_audiotest)
		TEST_ADD(GstTestSuite::start_stop_8ch_comp_rtp_audiotest)
		TEST_ADD(GstTestSuite::start_8ch_comp_rtp_audiofile)
		TEST_ADD(GstTestSuite::stop_8ch_comp_rtp_audiofile)
		TEST_ADD(GstTestSuite::start_stop_8ch_comp_rtp_audiofile)
#if VIDEO
		TEST_ADD(GstTestSuite::start_8ch_comp_rtp_audiofile_dv)
		TEST_ADD(GstTestSuite::stop_8ch_comp_rtp_audiofile_dv)
		TEST_ADD(GstTestSuite::start_stop_8ch_comp_rtp_audiofile_dv)
#endif                          // RTP && VIDEO && AUDIO
#endif                          // RTP  && AUDIO
#endif                          // AUDIO
		/*----------------------------------------------*/
		/*      SANDBOX                                 */
		/*----------------------------------------------*/
	}

	void set_id(int id);

// some tests

protected:
	virtual void setup();           // setup resources common to all tests
	virtual void tear_down();       // destroy common resources

private:
	int id_;

	void init_test();

	void start_test_video();
	void stop_test_video();
	void start_stop_test_video();

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

	void start_8ch_comp_rtp_audiofile_dv();
	void stop_8ch_comp_rtp_audiofile_dv();
	void start_stop_8ch_comp_rtp_audiofile_dv();
};

#if BLOCKING
#define BLOCK() std::cout.flush();                              \
    std::cout << __FILE__ << ":" << __LINE__        \
              << ": blocking, enter any key." << std::endl;   \
    std::cin.get()
#else
#define BLOCK()
#endif

#endif // _GST_TEST_SUITE_H_
