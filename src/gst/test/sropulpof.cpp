
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

const char *ROOM_A_IP = "10.10.10.188"; // Room A
const char *ROOM_B_IP = "10.10.10.190";	// Room B

const short Demo::NUM_CHANNELS = 4;

Sro::Sro(short pid, long txPort, long rxPort, const char *localIp, const char *remoteIp) 
    : Demo(pid), txPort_(txPort), rxPort_(rxPort), localIp_(localIp), remoteIp_(remoteIp)
{
    if (pid_ != 0 && pid_ != 1)
        THROW_ERROR(usage());
}

Pul::Pul(short pid) : Demo(pid)
{
    if (pid_ != 0 && pid_ != 1)
        THROW_ERROR(usage());
}

Pof::Pof(short pid) : Demo(pid)
{
    if (pid_ != 0 && pid_ != 1)
        THROW_ERROR(usage());
}

// 2 way audio
short Sro::run()
{
    if (pid_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(Factories::buildAudioReceiver(localIp_, rxPort_));
        aRx->start();
        
        BLOCK();
        assert(aRx->isPlaying());

        aRx->stop();
    } 
    else 
    {
        AudioConfig aConfig("jackaudiosrc", Demo::NUM_CHANNELS);
        std::auto_ptr<AudioSender> aTx(Factories::buildAudioSender(aConfig, remoteIp_, txPort_));
        aTx->start();
        assert(tcpSendCaps(ROOM_A_IP, Ports::CAPS_PORT, aTx->getCaps()));
        
        BLOCK();
        assert(aTx->isPlaying());

        aTx->stop();
    }
    return 0;
}

// One way video
short Pul::run()
{
    if (pid_ == 0) {
        std::auto_ptr<VideoReceiver> vRx(Factories::buildVideoReceiver(ROOM_A_IP));
        vRx->start();
        BLOCK();
        assert(vRx->isPlaying());
        vRx->stop();
    }
    else {
        VideoConfig vConfig("v4l2src"); 
        std::auto_ptr<VideoSender> vTx(Factories::buildVideoSender(vConfig, ROOM_A_IP));
        vTx->start();
        
        BLOCK();
        assert(vTx->isPlaying());
        vTx->stop();
    }
    return 0;
}


// One Way audio
short Pof::run()
{
    if (pid_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(Factories::buildAudioReceiver(ROOM_B_IP));
        aRx->start();
        
        BLOCK();
        assert(aRx->isPlaying());

        aRx->stop();
    }
    else {
        AudioConfig aConfig("jackaudiosrc", Demo::NUM_CHANNELS);
        std::auto_ptr<AudioSender> aTx(Factories::buildAudioSender(aConfig, ROOM_B_IP));
        aTx->start();
        assert(tcpSendCaps(ROOM_B_IP, Ports::CAPS_PORT, aTx->getCaps()));
        
        BLOCK();
        assert(aTx->isPlaying());

        aTx->stop();
    }
    return 0;
}


int mainSro(int argc, char **argv)
{
    int pid;
    long rxPort, txPort;
    std::string localIp, remoteIp;

    if (argc == 6)
    {
        pid = atoi(argv[1]);
        txPort = atoi(argv[2]);
        rxPort = atoi(argv[3]);
        localIp = argv[4];
        remoteIp = argv[5];
    }
    else
        THROW_ERROR(Sro::usage());

    Sro sro(pid, txPort, rxPort, localIp.c_str(), remoteIp.c_str());

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;

    try {
        return sro.run();
    }
    catch (Except e)
    {
        std::cerr << e.msg_;
        return 1;
    }
}


int mainPul(int argc, char **argv)
{
    int pid;
    if (argc > 1)
        pid = atoi(argv[1]);
    else
        THROW_ERROR(Pul::usage());

    Pul pul(pid);

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;

    try {
        return pul.run();
    }
    catch (Except e)
    {
        std::cerr << e.msg_;
        return 1;
    }
}


int mainPof(int argc, char **argv)
{
    char pid;
    if (argc > 1)
        pid = atoi(argv[1]);
    else
        THROW_ERROR(Pof::usage());

    Pof pof(pid);

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

