
// audioTestSuite.cpp
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
#include "audioTestSuite.h"
#include "audioLocal.h"
#include "audioConfig.h"

void AudioTestSuite::start_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();
    TEST_ASSERT(tx.stop());

    BLOCK();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_2ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_2ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_2ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_6ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_6ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_6ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}

#if 0
void AudioTestSuite::start_8ch_alsa()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_alsa()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_alsa()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}
#endif


void AudioTestSuite::start_8ch_jack()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioLocal tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_jack()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioLocal tx(config);
    TEST_ASSERT(tx.init());

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_jack()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioLocal tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_comp_audiofile()
{
    int numChannels = 8;

    if (id_ == 1)
        return;
    AudioConfig config("filesrc", audioFilename_, numChannels);
    AudioLocal tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_comp_audiofile()
{
    int numChannels = 8;
    if (id_ == 1)
        return;
    AudioConfig config("filesrc", audioFilename_, numChannels);
    AudioLocal tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_comp_audiofile()
{
    int numChannels = 8;
    if (id_ == 1)
        return;
    AudioConfig config("filesrc", audioFilename_, numChannels);
    AudioLocal tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_audio_dv()
{
    int numChannels = 2;
    if (id_ == 1)
        return;
    AudioConfig config("dv1394src", numChannels);
    AudioLocal tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_audio_dv()
{
    int numChannels = 2;
    if (id_ == 1)
        return;
    AudioConfig config("dv1394src", numChannels);
    AudioLocal tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(!tx.isPlaying());

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_audio_dv()
{
    int numChannels = 2;
    if (id_ == 1)
        return;
    AudioConfig config("dv1394src", numChannels);
    AudioLocal tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


int mainAudioTestSuite(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "audioTester <0/1>" << std::endl;
        return 1;
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    AudioTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


