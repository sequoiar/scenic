
// syncTestSuite.cpp
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

#include <cpptest.h>
#include <iostream>
#include <cstdlib>
#include "syncTestSuite.h"
#include "videoConfig.h"
#include "videoLocal.h"
#include "audioLocal.h"
#include "audioConfig.h"
#include "logWriter.h"

/*----------------------------------------------*/ 
/*----------------------------------------------*/ 
/* Unit tests                                   */
/*----------------------------------------------*/ 

void SyncTestSuite::start_jack_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioConfig aConfig("jackaudiosrc", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();


    aTx.start();
    vTx.start();

    BLOCK();
    TEST_ASSERT(aTx.isPlaying());
    TEST_ASSERT(vTx.isPlaying());
}


void SyncTestSuite::stop_jack_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioConfig aConfig("jackaudiosrc", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();

    BLOCK();

    aTx.stop();
    vTx.stop();

    TEST_ASSERT(!aTx.isPlaying());
    TEST_ASSERT(!vTx.isPlaying());
}


void SyncTestSuite::start_stop_jack_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioConfig aConfig("jackaudiosrc", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();

    aTx.start();
    vTx.start();

    BLOCK();

    TEST_ASSERT(aTx.isPlaying());
    TEST_ASSERT(vTx.isPlaying());

    aTx.stop();
    vTx.stop();

    TEST_ASSERT(!aTx.isPlaying());
    TEST_ASSERT(!vTx.isPlaying());
}

void SyncTestSuite::start_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioConfig aConfig("dv1394src", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();


    aTx.start();
    vTx.start();

    BLOCK();
    TEST_ASSERT(aTx.isPlaying());
    TEST_ASSERT(vTx.isPlaying());
}


void SyncTestSuite::stop_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioConfig aConfig("dv1394src", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();

    BLOCK();

    aTx.stop();
    vTx.stop();

    TEST_ASSERT(!aTx.isPlaying());
    TEST_ASSERT(!vTx.isPlaying());
}


void SyncTestSuite::start_stop_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioConfig aConfig("dv1394src", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();

    aTx.start();
    vTx.start();

    BLOCK();

    TEST_ASSERT(aTx.isPlaying());
    TEST_ASSERT(vTx.isPlaying());

    aTx.stop();
    vTx.stop();

    TEST_ASSERT(!aTx.isPlaying());
    TEST_ASSERT(!vTx.isPlaying());
}


void SyncTestSuite::start_audiotest_videotest()
{
    int numChannels = 8;

    if (id_ == 1)
        return;

    AudioConfig aConfig("audiotestsrc", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("videotestsrc");
    VideoLocal vTx(vConfig);
    vTx.init();


    aTx.start();
    vTx.start();

    BLOCK();
    TEST_ASSERT(aTx.isPlaying());
    TEST_ASSERT(vTx.isPlaying());
}


void SyncTestSuite::stop_audiotest_videotest()
{
    int numChannels = 8;

    if (id_ == 1)
        return;

    AudioConfig aConfig("audiotestsrc", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("videotestsrc");
    VideoLocal vTx(vConfig);
    vTx.init();

    BLOCK();

    aTx.stop();
    vTx.stop();

    TEST_ASSERT(!aTx.isPlaying());
    TEST_ASSERT(!vTx.isPlaying());
}


void SyncTestSuite::start_stop_audiotest_videotest()
{
    int numChannels = 8;

    if (id_ == 1)
        return;

    AudioConfig aConfig("dv1394src", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();

    aTx.start();
    vTx.start();

    BLOCK();

    TEST_ASSERT(aTx.isPlaying());
    TEST_ASSERT(vTx.isPlaying());

    aTx.stop();
    vTx.stop();

    TEST_ASSERT(!aTx.isPlaying());
    TEST_ASSERT(!vTx.isPlaying());
}




int mainSyncTestSuite(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "syncTester <0/1>" << std::endl;
        return 1;
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    SyncTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    try {
        return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    catch (Except e)
    {
        std::cerr << e.msg_;
        return 1;
    }
}

