/* syncTestSuiteRtp.cpp
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

// for testing stopped pipelines
static std::auto_ptr<AudioReceiver> 
buildDeadAudioReceiver(const char * sink = audiofactory::A_SINK)
{
    AudioSinkConfig aConfig(sink);
    ReceiverConfig rConfig(audiofactory::A_CODEC, ports::IP, ports::A_PORT, "");
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}



/*----------------------------------------------*/ 
/* Unit tests                                   */
/*----------------------------------------------*/ 


void SyncTestSuiteRtp::start_jack_v4l()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver(ports::IP, "h264", ports::V_PORT, 0, "xvimagesink"));
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver(ports::IP, "raw", ports::A_PORT));
        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig, ports::IP, "h264", ports::V_PORT));

        AudioSourceConfig aConfig("jackaudiosrc", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig, ports::IP, "raw", ports::A_PORT));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void SyncTestSuiteRtp::stop_jack_v4l()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildDeadAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("jackaudiosrc", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));

        VideoSourceConfig vConfig("v4l2src");
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_jack_v4l()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("jackaudiosrc", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig, ports::IP, "raw", ports::A_PORT));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));


        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_jack_v4l_vorbis()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver(ports::IP, "vorbis", ports::A_PORT));
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver(ports::IP, "h264", ports::V_PORT, 0, "xvimagesink"));
        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig, ports::IP, "h264", ports::V_PORT));

        AudioSourceConfig aConfig("jackaudiosrc", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig, ports::IP, "vorbis", ports::A_PORT));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_jack_v4l_vorbis()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("v4l2src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("jackaudiosrc", numChannels);

        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig, ports::IP, "vorbis", ports::A_PORT));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));


        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_8ch_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        playback::pause();
        
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("filesrc", audioFilename_, numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::pause();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));

        VideoSourceConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));
        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());
    }
}


void SyncTestSuiteRtp::stop_8ch_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildDeadAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("filesrc", audioFilename_, numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));

        VideoSourceConfig vConfig("dv1394src");
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_8ch_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        playback::start();
        
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("filesrc", audioFilename_, numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
    
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));

        VideoSourceConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        playback::start();
        

        BLOCK();

        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("dv1394src", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));

        BLOCK();

        TEST_ASSERT(playback::isPlaying());
    }
}


void SyncTestSuiteRtp::stop_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildDeadAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("dv1394src", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));

        VideoSourceConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {

        VideoSourceConfig vConfig("dv1394src"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("dv1394src", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));
        //usleep(100000); // GIVE receiver chance to start waiting

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_audiotest_videotest()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        playback::start();
        
        BLOCK();

        TEST_ASSERT(playback::isPlaying());
    }
    else {
        VideoSourceConfig vConfig("videotestsrc"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("audiotestsrc", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));

        //usleep(100000); // FIXME: this is all kinds of bad, GIVE receiver chance to start waiting

        BLOCK();

        TEST_ASSERT(playback::isPlaying());
    }
}


void SyncTestSuiteRtp::stop_audiotest_videotest()
{
    int numChannels = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> aRx(buildDeadAudioReceiver());
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("audiotestsrc", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));

        VideoSourceConfig vConfig("videotestsrc"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        BLOCK();

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
}


void SyncTestSuiteRtp::start_stop_audiotest_videotest()
{
    int numChannels = 2;

    if (id_ == 0) {
        std::auto_ptr<VideoReceiver> vRx(videofactory::buildVideoReceiver());
        std::auto_ptr<AudioReceiver> aRx(audiofactory::buildAudioReceiver());
        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();

        TEST_ASSERT(!playback::isPlaying());
    }
    else {

        VideoSourceConfig vConfig("videotestsrc"); 
        std::auto_ptr<VideoSender> vTx(videofactory::buildVideoSender(vConfig));

        AudioSourceConfig aConfig("audiotestsrc", numChannels);
        std::auto_ptr<AudioSender> aTx(audiofactory::buildAudioSender(aConfig));
        playback::start();
        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::CAPS_PORT, aTx->getCaps()));
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

