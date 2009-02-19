/* syncTestSuite.cpp
 * Copyright (C) 2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include <cpptest.h>
#include <cstdlib>
#include "syncTestSuite.h"
#include "videoConfig.h"
#include "videoLocal.h"
#include "audioLocal.h"
#include "audioConfig.h"
#include "videoSink.h"
#include "playback.h"

/*----------------------------------------------*/ 
/* Unit tests                                   */
/*----------------------------------------------*/ 


void SyncTestSuite::start_stop_jack_audio_dv_video()
{
    const int NUM_CHANNELS = 2;

    if (id_ == 1)
        return;

    AudioSourceConfig audioSrcConfig("jackaudiosrc", NUM_CHANNELS);
    AudioSinkConfig audioSinkConfig("jackaudiosink");
    AudioLocal aTx(audioSrcConfig, audioSinkConfig);
    aTx.init();

    VideoSourceConfig videoSrcConfig("dv1394src", 3000000);
    VideoSinkConfig videoSinkConfig("xvimagesink");
    VideoLocal vTx(videoSrcConfig, videoSinkConfig);
    vTx.init();

    playback::start();
    
    BLOCK();

    TEST_ASSERT(playback::isPlaying());

    playback::stop();

    TEST_ASSERT(!playback::isPlaying());
}



void SyncTestSuite::start_stop_dv_audio_dv_video()
{
    const int NUM_CHANNELS = 2;

    if (id_ == 1)
        return;

    AudioSourceConfig audioSrcConfig("dv1394src", NUM_CHANNELS);
    AudioSinkConfig audioSinkConfig("jackaudiosink");
    AudioLocal aTx(audioSrcConfig, audioSinkConfig);
    aTx.init();

    VideoSourceConfig videoSrcConfig("dv1394src", 3000000);
    VideoSinkConfig videoSinkConfig("xvimagesink");
    VideoLocal vTx(videoSrcConfig, videoSinkConfig);
    vTx.init();

    playback::start();
    
    BLOCK();

    TEST_ASSERT(playback::isPlaying());

    playback::stop();

    TEST_ASSERT(!playback::isPlaying());
}



void SyncTestSuite::start_stop_audiotest_videotest()
{
    const int NUM_CHANNELS = 8;

    if (id_ == 1)
        return;

    AudioSourceConfig audioSrcConfig("audiotestsrc", NUM_CHANNELS);
    AudioSinkConfig audioSinkConfig("jackaudiosink");
    AudioLocal aTx(audioSrcConfig, audioSinkConfig);
    aTx.init();

    VideoSourceConfig videoSrcConfig("dv1394src", 3000000);
    VideoSinkConfig videoSinkConfig("xvimagesink");
    VideoLocal vTx(videoSrcConfig, videoSinkConfig);
    vTx.init();

    playback::start();
    
    BLOCK();

    TEST_ASSERT(playback::isPlaying());

    playback::stop();

    TEST_ASSERT(!playback::isPlaying());
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

