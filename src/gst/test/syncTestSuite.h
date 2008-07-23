
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

#include <cpptest.h>

#define DV 1

class SyncTestSuite : public Test::Suite
{
public:

    SyncTestSuite()
    {
#if DV
        TEST_ADD(SyncTestSuite::start_8ch_comp_rtp_audiofile_dv)
        TEST_ADD(SyncTestSuite::stop_8ch_comp_rtp_audiofile_dv)
        TEST_ADD(SyncTestSuite::start_stop_8ch_comp_rtp_audiofile_dv)
#endif  // DV

        TEST_ADD(SyncTestSuite::sync);
    }

    void set_id(int id);

// some tests

protected:
    virtual void setup();           // setup resources common to all tests
    virtual void tear_down();       // destroy common resources

private:
    int id_;

    void start_8ch_comp_rtp_audiofile_dv();
    void stop_8ch_comp_rtp_audiofile_dv();
    void start_stop_8ch_comp_rtp_audiofile_dv();

    void sync();
};

#endif // _SYNC_TEST_SUITE_H_

