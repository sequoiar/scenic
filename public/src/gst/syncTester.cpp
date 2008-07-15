
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

SyncTester::SyncTester(const VideoConfig &vConf, const AudioConfig &aConf) : vConfig_(vConf), aConfig_(aConf)
{
}

SyncTesterSend::SyncTesterSend() : SyncTester(VideoConfig("videotestsrc", "h264", MY_ADDRESS, V_PORT), 
        AudioConfig("audiotestsrc", 8, "vorbisenc", MY_ADDRESS, A_PORT)), 
        vTx_(vConfig_),
        aTx_(aConfig_)
{
}

SyncTesterReceive::SyncTesterReceive() : SyncTester(VideoConfig("h264", V_PORT), 
        AudioConfig(8, "vorbisdec", A_PORT)), 
        vRx_(vConfig_),
        aRx_(aConfig_)
{
}

void SyncTesterSend::run()
{
        vTx_.init();
        aTx_.init();

        assert(aTx_.start());
        assert(vTx_.start());

        BLOCK();
        assert(vTx_.isPlaying());
        assert(aTx_.isPlaying());
}

void SyncTesterReceive::run()
{
        vRx_.init();
        aRx_.init();

        assert(vRx_.start());
        assert(aRx_.start());

        BLOCK();
        assert(vRx_.isPlaying());
        assert(aRx_.isPlaying());
}
