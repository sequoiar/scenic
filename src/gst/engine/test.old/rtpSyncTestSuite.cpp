/* syncTestSuiteRtp.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
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

#define USE_SMART_PTR

#include "util.h"

#include <cpptest.h>
#include <cstdlib>
#include "rtpSyncTestSuite.h"
#include "videoSender.h"
#include "videoReceiver.h"
#include "audioSender.h"
#include "audioReceiver.h"
#include "remoteConfig.h"
#include "playback.h"

#include "gst/videoFactory.h"
#include "gst/audioFactory.h"
#include "tcp/singleBuffer.h"

/*----------------------------------------------*/ 
/* Helper functions                             */
/*----------------------------------------------*/ 

static const int V_BITRATE = 3000000;

/*----------------------------------------------*/ 
/* Unit tests                                   */
/*----------------------------------------------*/ 


void SyncTestSuiteRtp::start_stop_jack_v4l()
{
    int numChannels = 8;

    if (id_ == 0) {
        boost::shared_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        boost::shared_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src", V_BITRATE); 
        boost::shared_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("jackaudiosrc", numChannels);
        boost::shared_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig, ports::IP, "raw", ports::A_PORT));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::VIDEO_CAPS_PORT, videofactory::MSG_ID, vTx->getCaps()));
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, aTx->getCaps()));


        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}



void SyncTestSuiteRtp::start_stop_jack_v4l_vorbis()
{
    int numChannels = 8;

    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        boost::shared_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src", V_BITRATE); 
        boost::shared_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("jackaudiosrc", numChannels);

        boost::shared_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig, ports::IP, "vorbis", ports::A_PORT));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, aTx->getCaps()));
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::VIDEO_CAPS_PORT, videofactory::MSG_ID, vTx->getCaps()));


        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}



void SyncTestSuiteRtp::start_stop_8ch_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        
        boost::shared_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("filesrc", audioFilename_, numChannels);
        boost::shared_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
    

        VideoSourceConfig vConfig("dv1394src", V_BITRATE); 
        boost::shared_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, aTx->getCaps()));
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::VIDEO_CAPS_PORT, videofactory::MSG_ID, vTx->getCaps()));

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 0) {
        boost::shared_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        boost::shared_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {

        VideoSourceConfig vConfig("dv1394src", V_BITRATE); 
        boost::shared_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("dv1394src", numChannels);
        boost::shared_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::VIDEO_CAPS_PORT, videofactory::MSG_ID, vTx->getCaps()));
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, aTx->getCaps()));

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}



void SyncTestSuiteRtp::start_stop_audiotest_videotest()
{
    int numChannels = 2;

    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        boost::shared_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {

        VideoSourceConfig vConfig("videotestsrc", V_BITRATE); 
        boost::shared_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("audiotestsrc", numChannels);
        boost::shared_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, aTx->getCaps()));
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::VIDEO_CAPS_PORT, videofactory::MSG_ID, vTx->getCaps()));
        //usleep(100000); // GIVE receiver chance to start waiting

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


int mainRtpSyncTestSuite(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "syncTesterRtp <0/1>" << std::endl;
        return 1;
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    SyncTestSuiteRtp tester;
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

