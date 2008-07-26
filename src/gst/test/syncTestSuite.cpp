
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

#include "syncTestSuite.h"
#include "videoSender.h"
#include "videoConfig.h"
#include "videoReceiver.h"
#include "audioSender.h"
#include "audioReceiver.h"
#include "audioConfig.h"


void SyncTestSuite::start_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;
    int vPort = 10010;
    int aPort = vPort + 1000;

    if (id_ == 0) {
        AudioConfig aConfig(numChannels, "vorbisdec", aPort);
        AudioReceiver aRx(aConfig);
        aRx.init();

        VideoConfig vConfig("h264", vPort);
        VideoReceiver vRx(vConfig);
        vRx.init();


        TEST_ASSERT(aRx.start());
        TEST_ASSERT(vRx.start());

        BLOCK();
        TEST_ASSERT(aRx.isPlaying());
        TEST_ASSERT(vRx.isPlaying());
    }
    else{
        AudioConfig aConfig("filesrc", numChannels, "vorbisenc", MY_ADDRESS, aPort);
        AudioSender aTx(aConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src", "h264", MY_ADDRESS, vPort);
        VideoSender vTx(vConfig);
        vTx.init();

        TEST_ASSERT(aTx.start());
        TEST_ASSERT(vTx.start());

        BLOCK();
        TEST_ASSERT(aTx.isPlaying());
        TEST_ASSERT(vTx.isPlaying());
    }
}

void SyncTestSuite::stop_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;
    int vPort = 10010;
    int aPort = vPort + 1000;

    if (id_ == 0) {
        AudioConfig aConfig(numChannels, "vorbisdec", aPort);
        AudioReceiver aRx(aConfig);
        aRx.init();

        VideoConfig vConfig("h264", vPort);
        VideoReceiver vRx(vConfig);
        vRx.init();

        BLOCK();

        TEST_ASSERT(aRx.stop());
        TEST_ASSERT(vRx.stop());

        TEST_ASSERT(!aRx.isPlaying());
        TEST_ASSERT(!vRx.isPlaying());
    }
    else{
        AudioConfig aConfig("filesrc", numChannels, "vorbisenc", MY_ADDRESS, aPort);
        AudioSender aTx(aConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src", "h264", MY_ADDRESS, vPort);
        VideoSender vTx(vConfig);
        vTx.init();


        BLOCK();

        TEST_ASSERT(aTx.stop());
        TEST_ASSERT(vTx.stop());

        TEST_ASSERT(!aTx.isPlaying());
        TEST_ASSERT(!vTx.isPlaying());
    }
}

void SyncTestSuite::start_stop_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;
    int vPort = 10010;
    int aPort = vPort + 1000;

    if (id_ == 0) {
        AudioConfig aConfig(numChannels, "vorbisdec", aPort);
        AudioReceiver aRx(aConfig);
        aRx.init();

        VideoConfig vConfig("h264", vPort);
        VideoReceiver vRx(vConfig);
        vRx.init();

        TEST_ASSERT(aRx.start());
        TEST_ASSERT(vRx.start());

        BLOCK();

        TEST_ASSERT(aRx.isPlaying());
        TEST_ASSERT(vRx.isPlaying());

        TEST_ASSERT(aRx.stop());
        TEST_ASSERT(vRx.stop());

        TEST_ASSERT(!aRx.isPlaying());
        TEST_ASSERT(!vRx.isPlaying());
    }
    else{
        AudioConfig aConfig("filesrc", numChannels, "vorbisenc", MY_ADDRESS, aPort);
        AudioSender aTx(aConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src", "h264", MY_ADDRESS, vPort);
        VideoSender vTx(vConfig);
        vTx.init();

        TEST_ASSERT(aTx.start());
        TEST_ASSERT(vTx.start());

        BLOCK();

        TEST_ASSERT(aTx.isPlaying());
        TEST_ASSERT(vTx.isPlaying());

        TEST_ASSERT(aTx.stop());
        TEST_ASSERT(vTx.stop());

        TEST_ASSERT(!aTx.isPlaying());
        TEST_ASSERT(!vTx.isPlaying());
    }
}

void SyncTestSuite::sync()
{
    if (id_ == 0)
    {
        AudioConfig aConfig(NUM_CHANNELS, "vorbisdec", A_PORT);
        AudioReceiver aRx(aConfig);
        aRx.init();

        VideoConfig vConfig("h264", V_PORT);
        VideoReceiver vRx(vConfig);
        vRx.init();

        TEST_ASSERT(aRx.start());
        TEST_ASSERT(vRx.start());

        BLOCK();

        TEST_ASSERT(aRx.isPlaying());
        TEST_ASSERT(vRx.isPlaying());

        TEST_ASSERT(aRx.stop());
        TEST_ASSERT(vRx.stop());

        TEST_ASSERT(!aRx.isPlaying());
        TEST_ASSERT(!vRx.isPlaying());
    }
    else{
        AudioConfig aConfig("audiotestsrc", NUM_CHANNELS, "vorbisenc", MY_ADDRESS
                            ,A_PORT);
        AudioSender aTx(aConfig);
        aTx.init();

        VideoConfig vConfig("videotestsrc", "h264", MY_ADDRESS, V_PORT);
        VideoSender vTx(vConfig);
        vTx.init();

        TEST_ASSERT(aTx.start());
        TEST_ASSERT(vTx.start());

        BLOCK();

        TEST_ASSERT(aTx.isPlaying());
        TEST_ASSERT(vTx.isPlaying());

        TEST_ASSERT(aTx.stop());
        TEST_ASSERT(vTx.stop());

        TEST_ASSERT(!aTx.isPlaying());
        TEST_ASSERT(!vTx.isPlaying());
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << "syncTester <0/1>" << std::endl;
        exit(1);
    }
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    SyncTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

