
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

#include "factories.h"
#include "eventLoop.h"
#include <cassert>
#include <cstdlib>
#include "sropulpof.h"

const short Pof::NUM_CHANNELS = 2;

Pof::Pof(char pid, const char *ip, const char *videoCodec, const char *audioCodec, long videoPort, long audioPort)
: pid_(pid), ip_(ip), videoCodec_(videoCodec), audioCodec_(audioCodec), videoPort_(videoPort), audioPort_(audioPort)
{
    if (pid_ != 'r' && pid_ != 's')
        THROW_ERROR("Invalid pid");
}

// 2way audio and video
short Pof::run()
{
    if (pid_ == 'r') {
        std::auto_ptr<AudioReceiver> aRx(Factories::buildAudioReceiver(ip_, audioCodec_, audioPort_));
        aRx->start();
        
        std::auto_ptr<VideoReceiver> vRx(Factories::buildVideoReceiver(ip_, videoCodec_, videoPort_));
        vRx->start();
        
        BLOCK();
        assert(aRx->isPlaying());
        assert(vRx->isPlaying());

        aRx->stop();
        vRx->stop();
    }
    else {
        AudioConfig aConfig("jackaudiosrc", Pof::NUM_CHANNELS);
        std::auto_ptr<AudioSender> aTx(Factories::buildAudioSender(aConfig, ip_, audioCodec_, audioPort_));
        aTx->start();
        assert(tcpSendCaps(ip_, Ports::CAPS_PORT, aTx->getCaps()));

        VideoConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> vTx(Factories::buildVideoSender(vConfig, ip_, videoCodec_, videoPort_));
        vTx->start();
        
        BLOCK();
        assert(aTx->isPlaying());
        assert(vTx->isPlaying());

        aTx->stop();
        vTx->stop();
    }
    return 0;
}


#include "gutil/optionArgs.h"
int mainPof(int argc, char **argv)
{
    char pid;
    bool send = false;
    bool recv = false;
    OptionArgs options;
    char *ip =0;
    char *videoCodec = 0;
    char *audioCodec = 0;
    int audioPort = 0;
    int videoPort = 0;

    options.add(new StringArg(&ip, "address", 'i', "address", "provide ip address"));
    options.add(new StringArg(&videoCodec, "videocodec", 'v', "videocodec", "h264"));
    options.add(new StringArg(&audioCodec, "audiocodec", 'a', "audiocodec", "vorbis raw mp3"));
    options.add(new IntArg(&audioPort, "audioport", 't', "audioport", ""));
    options.add(new IntArg(&videoPort, "videoport", 'p', "videoport", ""));
    options.add(new BoolArg(&send,"sender", 's', "sender"));
    options.add(new BoolArg(&recv,"receiver", 'r', "receiver"));

    options.parse(argc, argv);

        pid = send ? 's' : 'r';
/*
    if (ip != 0 && (send || recv) )
        pid = send ? 's' : 'r';
    else
        THROW_ERROR("Check yourself before you wreck yourself");
*/

    Pof pof(pid, ip, videoCodec, audioCodec, videoPort, audioPort);

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

