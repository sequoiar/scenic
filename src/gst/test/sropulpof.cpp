
// sropulpof.cpp
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

#include "sropulpof.h"
#include "factories.h"
#include "eventLoop.h"
#include <cassert>
#include <cstdlib>

const char *POF_0_IP = "10.10.10.238";
const char *POF_1_IP = "10.10.10.189";

const short Sropulpof::NUM_CHANNELS = 4;

Sropulpof::Sropulpof(short pid) : pid_(pid)
{
    if (pid_ != 0 && pid_ != 1)
        THROW_ERROR(Sropulpof::usage());
}

short Sropulpof::run()
{
	// VIDEO_SENDER_ROOM
    if (pid_ == 0) {
        VideoConfig vConfig("v4l2src"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig, POF_1_IP));
        vTx->start();

        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver(POF_0_IP));
        aRx->pause();
        aRx->start();
        

        BLOCK();
        assert(aRx->isPlaying());
        assert(vTx->isPlaying());

        aRx->stop();
        vTx->stop();
    }
    else {
	    // AUDIO_SENDER_ROOM
        AudioConfig aConfig("jackaudiosrc", Sropulpof::NUM_CHANNELS);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig, POF_0_IP));
        aTx->start();
        assert(tcpSendCaps(POF_0_IP, Ports::CAPS_PORT, aTx->getCaps()));
        
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver(POF_1_IP));
        usleep(100000);
        vRx->start();

        BLOCK();
        assert(aTx->isPlaying());
        assert(vRx->isPlaying());

        aTx->stop();
        vRx->stop();
    }
    return 0;
}




int mainSropulpof(int argc, char **argv)
{
    int pid;
    if (argc > 1)
        pid = atoi(argv[1]);
    else
        THROW_ERROR(Sropulpof::usage());

    Sropulpof pof(pid);

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;

    try {
        return pof.run();
    }
    catch (Except e)
    {
        std::cerr << e.msg_;
        return 1;
    }
}

