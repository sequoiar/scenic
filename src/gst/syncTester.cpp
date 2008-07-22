
// syncTester.cpp
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

#include <iostream>

#define BLOCK() std::cout.flush();                              \
    std::cout << __FILE__ << ":" << __LINE__        \
              << ": blocking, enter any key." << std::endl;   \
    std::cin.get()

#include <cassert>
#include "defaultAddresses.h"
#include "syncTester.h"

const int SyncTester::V_PORT = 10010;
const int SyncTester::A_PORT = 11010;
const int SyncTester::NUM_CHANNELS = 8;

void SyncTesterSend::run()
{
    VideoConfig vConfig("videotestsrc", "h264", MY_ADDRESS, V_PORT);
    VideoSender vTx(vConfig);
    vTx.init();
    
    AudioConfig aConfig("audiotestsrc", NUM_CHANNELS, "vorbisenc", MY_ADDRESS, A_PORT);
    AudioSender aTx(aConfig);
    aTx.init();

    assert(vTx.start());
    assert(aTx.start());

    BLOCK();

    assert(vTx.isPlaying());
    assert(aTx.isPlaying());
    
    assert(vTx.stop());
    assert(aTx.stop());

    assert(!vTx.isPlaying());
    assert(!aTx.isPlaying());
}

void SyncTesterReceive::run()
{
    VideoConfig vConfig("h264", V_PORT);
    VideoReceiver vRx(vConfig);
    vRx.init();

    AudioConfig aConfig(NUM_CHANNELS, "vorbisdec", A_PORT);
    AudioReceiver aRx(aConfig);
    aRx.init();

    assert(vRx.start());
    assert(aRx.start());

    BLOCK();

    assert(vRx.isPlaying());
    assert(aRx.isPlaying());

    assert(vRx.stop());
    assert(aRx.stop());
        
    assert(!vRx.isPlaying());
    assert(!aRx.isPlaying());
}

