/* sropulpof.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

#include "gst/factories.h"
//#include "eventLoop.h"
//#include "videoSink.h"
#include <cassert>
#include <cstdlib>
#include "logWriter.h"
#include "gutil.h"
#include "gst/msgThreadFactory.h"

#define BLOCK() gutil::runMainLoop(0);

namespace pof 
{
    short run(int argc, char **argv);
    const short NUM_CHANNELS = 2;
}

// 2way audio and video
short pof::run(int argc, char **argv)
{
    char pid;
    bool send = false;
    bool recv = false;
    bool full = false;
    OptionArgs options;
    char *ip = 0;
    char *videoCodec = 0;
    char *audioCodec = 0;
    char *videoSink = 0;
    int audioPort = 0;
    int videoPort = 0;
    char *videoDevice = 0;

    int screenNum = 0;
    

    options.add(new StringArg(&ip, "address", 'i', "address", "provide ip address"));
    options.add(new StringArg(&videoCodec, "videocodec", 'v', "videocodec", "h264"));
    options.add(new StringArg(&audioCodec, "audiocodec", 'a', "audiocodec", "vorbis raw mp3"));
    options.add(new StringArg(&videoSink, "videosink", 'k', "videosink", "xvimagesink glimagesink"));
    options.add(new IntArg(&audioPort, "audioport", 't', "audioport", "portnum"));
    options.add(new IntArg(&videoPort, "videoport", 'p', "videoport", "portnum"));
    options.add(new BoolArg(&send,"sender", 's', "sender"));
    options.add(new BoolArg(&recv,"receiver", 'r', "receiver"));
    options.add(new BoolArg(&full,"fullscreen", 'f', "default to fullscreen"));
    options.add(new StringArg(&videoDevice, "videoDevice", 'd', "device", "/dev/video0 /dev/video1"));
    options.add(new IntArg(&screenNum, "screenNum", 'n', "screenNum", "xinerama screen num"));

    options.parse(argc, argv);

    pid = send ? 's' : 'r';

    LOG_INFO("Built on " << __DATE__ << " at " << __TIME__);
    if (pid == 'r') {
        std::auto_ptr<AudioReceiver> aRx(factories::buildAudioReceiver(ip, audioCodec, audioPort));
        playback::start();
        std::auto_ptr<VideoReceiver> vRx(factories::buildVideoReceiver(ip, videoCodec, videoPort, screenNum, videoSink));
        if(full)
            vRx->getVideoSink()->makeFullscreen();

        BLOCK();
        assert(playback::isPlaying());

        playback::stop();
    }
    else {
        AudioSourceConfig aConfig("jackaudiosrc", pof::NUM_CHANNELS);
        std::auto_ptr<AudioSender> aTx(factories::buildAudioSender(aConfig, ip, audioCodec, audioPort));
        playback::start();
        assert(tcpSendBuffer(ip, ports::CAPS_PORT, aTx->getCaps()));
        VideoSourceConfig *vConfig; 

        if (videoDevice)
            vConfig = new VideoSourceConfig("v4l2src", videoDevice);
        else
            vConfig = new VideoSourceConfig("v4l2src");

        std::auto_ptr<VideoSender> vTx(factories::buildVideoSender(*vConfig, ip, videoCodec, videoPort));
        delete vConfig;

        BLOCK();
        assert(playback::isPlaying());

        playback::stop();
    }
    return 0;
}


int mainPof(int argc, char **argv)
{
    LOG_INFO("Built on " << __DATE__ << " at " << __TIME__);
    try {
        return pof::run(argc, argv);
    }
    catch (Except e)
    {
        //std::cerr << e.msg_;
        return 1;
    }
}

