
// syncTestSuiteRtp.cpp
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
#include "rtpSyncTestSuite.h"
#include "videoSender.h"
#include "videoConfig.h"
#include "videoReceiver.h"
#include "videoLocal.h"
#include "audioSender.h"
#include "audioLocal.h"
#include "audioReceiver.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "hostIP.h"
#include <sstream>

#include "capsHelper.h"

/*----------------------------------------------*/ 
/* Helper functions                             */
/*----------------------------------------------*/ 

static std::auto_ptr<AudioSender> buildAudioSender(const AudioConfig aConfig)
{
    SenderConfig rConfig("vorbis", get_host_ip(), GstTestSuite::A_PORT);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    assert(tx->init());
    return tx;
}


static std::auto_ptr<AudioReceiver> buildAudioReceiver()
{
    AudioReceiverConfig aConfig("jackaudiosink");
    ReceiverConfig rConfig("vorbis", get_host_ip(), GstTestSuite::A_PORT); 
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    assert(rx->init());
    return rx;
}


static std::auto_ptr<VideoReceiver> buildVideoReceiver()
{
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), GstTestSuite::V_PORT);
        std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
        assert(rx->init());
        return rx;
}


static std::auto_ptr<VideoSender> buildVideoSender(const VideoConfig vConfig)
{
        SenderConfig rConfig("h264", get_host_ip(), GstTestSuite::V_PORT);
        std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
        assert(tx->init());
        return tx;
}

/*----------------------------------------------*/ 
/* Unit tests                                   */
/*----------------------------------------------*/ 


void SyncTestSuiteRtp::start_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());
        
        TEST_ASSERT(tcpGetCaps(A_PORT + 100, *aRx));

        TEST_ASSERT(aRx->start());
        TEST_ASSERT(vRx->start());

        BLOCK();
        TEST_ASSERT(aRx->isPlaying());
        TEST_ASSERT(vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", audioFilename_, numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        TEST_ASSERT(aTx->start());
        //usleep(100000); // GIVE receiver chance to start waiting
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();
        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::stop_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());

        BLOCK();

        TEST_ASSERT(aRx->stop());
        TEST_ASSERT(vRx->stop());

        TEST_ASSERT(!aRx->isPlaying());
        TEST_ASSERT(!vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", audioFilename_, numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        BLOCK();

        TEST_ASSERT(aTx->stop());
        TEST_ASSERT(vTx->stop());

        TEST_ASSERT(!aTx->isPlaying());
        TEST_ASSERT(!vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());
        
        TEST_ASSERT(tcpGetCaps(A_PORT + 100, *aRx));

        TEST_ASSERT(aRx->start());
        TEST_ASSERT(vRx->start());

        BLOCK();

        TEST_ASSERT(aRx->isPlaying());
        TEST_ASSERT(vRx->isPlaying());

        TEST_ASSERT(aRx->stop());
        TEST_ASSERT(vRx->stop());

        TEST_ASSERT(!aRx->isPlaying());
        TEST_ASSERT(!vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", audioFilename_, numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        TEST_ASSERT(aTx->start());
        //usleep(100000); // GIVE receiver chance to start waiting
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();

        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());

        TEST_ASSERT(aTx->stop());
        TEST_ASSERT(vTx->stop());

        TEST_ASSERT(!aTx->isPlaying());
        TEST_ASSERT(!vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::start_dv_audio_dv_video_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());
        
        TEST_ASSERT(tcpGetCaps(A_PORT + 100, *aRx));

        TEST_ASSERT(aRx->start());
        TEST_ASSERT(vRx->start());
        
        BLOCK();

        TEST_ASSERT(aRx->isPlaying());
        TEST_ASSERT(vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        TEST_ASSERT(aTx->start());
        //usleep(100000); // GIVE receiver chance to start waiting
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();

        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::stop_dv_audio_dv_video_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());

        BLOCK();

        TEST_ASSERT(aRx->stop());
        TEST_ASSERT(vRx->stop());

        TEST_ASSERT(!aRx->isPlaying());
        TEST_ASSERT(!vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        BLOCK();

        TEST_ASSERT(aTx->stop());
        TEST_ASSERT(vTx->stop());

        TEST_ASSERT(!aTx->isPlaying());
        TEST_ASSERT(!vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_dv_audio_dv_video_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());
        
        TEST_ASSERT(tcpGetCaps(A_PORT + 100, *aRx));

        TEST_ASSERT(aRx->start());
        TEST_ASSERT(vRx->start());

        BLOCK();

        TEST_ASSERT(aRx->isPlaying());
        TEST_ASSERT(vRx->isPlaying());

        TEST_ASSERT(aRx->stop());
        TEST_ASSERT(vRx->stop());

        TEST_ASSERT(!aRx->isPlaying());
        TEST_ASSERT(!vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        TEST_ASSERT(aTx->start());
        //usleep(100000); // GIVE receiver chance to start waiting
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();

        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());

        TEST_ASSERT(aTx->stop());
        TEST_ASSERT(vTx->stop());

        TEST_ASSERT(!aTx->isPlaying());
        TEST_ASSERT(!vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::start_audiotest_videotest_rtp()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());
        
        TEST_ASSERT(tcpGetCaps(A_PORT + 100, *aRx));

        TEST_ASSERT(aRx->start());
        TEST_ASSERT(vRx->start());
        
        BLOCK();

        TEST_ASSERT(aRx->isPlaying());
        TEST_ASSERT(vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("videotestsrc"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        TEST_ASSERT(aTx->start());
        //usleep(100000); // FIXME: this is all kinds of bad, GIVE receiver chance to start waiting
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();

        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::stop_audiotest_videotest_rtp()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());

        BLOCK();

        TEST_ASSERT(aRx->stop());
        TEST_ASSERT(vRx->stop());

        TEST_ASSERT(!aRx->isPlaying());
        TEST_ASSERT(!vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("videotestsrc"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        BLOCK();

        TEST_ASSERT(aTx->stop());
        TEST_ASSERT(vTx->stop());

        TEST_ASSERT(!aTx->isPlaying());
        TEST_ASSERT(!vTx->isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_audiotest_videotest_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(buildVideoReceiver());
        
        TEST_ASSERT(tcpGetCaps(A_PORT + 100, *aRx));

        TEST_ASSERT(aRx->start());
        TEST_ASSERT(vRx->start());

        BLOCK();

        TEST_ASSERT(aRx->isPlaying());
        TEST_ASSERT(vRx->isPlaying());

        TEST_ASSERT(aRx->stop());
        TEST_ASSERT(vRx->stop());

        TEST_ASSERT(!aRx->isPlaying());
        TEST_ASSERT(!vRx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        std::auto_ptr<AudioSender> aTx(buildAudioSender(aConfig));

        VideoConfig vConfig("videotestsrc"); 
        std::auto_ptr<VideoSender> vTx(buildVideoSender(vConfig));

        TEST_ASSERT(aTx->start());
        //usleep(100000); // GIVE receiver chance to start waiting
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();

        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());

        TEST_ASSERT(aTx->stop());
        TEST_ASSERT(vTx->stop());

        TEST_ASSERT(!aTx->isPlaying());
        TEST_ASSERT(!vTx->isPlaying());
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

