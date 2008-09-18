
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
#include <cstdlib>
//#define USE_OSC
#include "syncTestSuite.h"
#include "videoSender.h"
#include "videoConfig.h"
#include "videoReceiver.h"
#include "videoLocal.h"
#include "audioSender.h"
#include "audioLocal.h"
#include "audioReceiver.h"
#include "audioLocalConfig.h"
#include "remoteConfig.h"
#include "hostIP.h"


void SyncTestSuite::start_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        RemoteReceiverConfig rAConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver aRx(aConfig, rAConfig);
        aRx.init();

        VideoReceiverConfig vConfig("xvimagesink");
        RemoteReceiverConfig rVConfig("h264", get_host_ip(), V_PORT); 
        VideoReceiver vRx(vConfig, rVConfig);
        vRx.init();


        TEST_ASSERT(aRx.start());
        TEST_ASSERT(vRx.start());

        BLOCK();
        TEST_ASSERT(aRx.isPlaying());
        TEST_ASSERT(vRx.isPlaying());
    }
    else {
        AudioLocalConfig aConfig("filesrc", fileLocation_, numChannels);
        RemoteSenderConfig rAConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender aTx(aConfig, rAConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src"); 
        RemoteSenderConfig rVConfig("h264", get_host_ip(), V_PORT);
        VideoSender vTx(vConfig, rVConfig);
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

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        RemoteReceiverConfig raConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver aRx(aConfig, raConfig);
        aRx.init();

        VideoReceiverConfig vConfig("xvimagesink");
        RemoteReceiverConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver vRx(vConfig, rvConfig);
        vRx.init();

        BLOCK();

        TEST_ASSERT(aRx.stop());
        TEST_ASSERT(vRx.stop());

        TEST_ASSERT(!aRx.isPlaying());
        TEST_ASSERT(!vRx.isPlaying());
    }
    else {
        AudioLocalConfig aConfig("filesrc", fileLocation_, numChannels);
        RemoteSenderConfig raConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender aTx(aConfig, raConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src");
        RemoteSenderConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoSender vTx(vConfig, rvConfig);
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

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        RemoteReceiverConfig raConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver aRx(aConfig, raConfig);
        aRx.init();

        VideoReceiverConfig vConfig("xvimagesink");
        RemoteReceiverConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver vRx(vConfig, rvConfig);
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
    else {
        AudioLocalConfig aConfig("filesrc", fileLocation_, numChannels);
        RemoteSenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender aTx(aConfig, rConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src");
        RemoteSenderConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoSender vTx(vConfig, rvConfig);
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


void SyncTestSuite::start_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioLocalConfig aConfig("dv1394src", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();


    TEST_ASSERT(aTx.start());
    TEST_ASSERT(vTx.start());

    BLOCK();
    TEST_ASSERT(aTx.isPlaying());
    TEST_ASSERT(vTx.isPlaying());
}


void SyncTestSuite::stop_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioLocalConfig aConfig("dv1394src", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
    vTx.init();

    BLOCK();

    TEST_ASSERT(aTx.stop());
    TEST_ASSERT(vTx.stop());

    TEST_ASSERT(!aTx.isPlaying());
    TEST_ASSERT(!vTx.isPlaying());
}


void SyncTestSuite::start_stop_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioLocalConfig aConfig("dv1394src", numChannels);
    AudioLocal aTx(aConfig);
    aTx.init();

    VideoConfig vConfig("dv1394src");
    VideoLocal vTx(vConfig);
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


void SyncTestSuite::start_dv_audio_dv_video_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        RemoteReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver aRx(aConfig, rConfig);
        aRx.init();

        VideoReceiverConfig vConfig("xvimagesink");
        RemoteReceiverConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver vRx(vConfig, rvConfig);
        vRx.init();


        TEST_ASSERT(aRx.start());
        TEST_ASSERT(vRx.start());

        BLOCK();
        TEST_ASSERT(aRx.isPlaying());
        TEST_ASSERT(vRx.isPlaying());
    }
    else {
        AudioLocalConfig aConfig("dv1394src", numChannels);
        RemoteSenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender aTx(aConfig, rConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src");
        RemoteSenderConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoSender vTx(vConfig, rvConfig);
        vTx.init();

        TEST_ASSERT(aTx.start());
        TEST_ASSERT(vTx.start());

        BLOCK();
        TEST_ASSERT(aTx.isPlaying());
        TEST_ASSERT(vTx.isPlaying());
    }
}


void SyncTestSuite::stop_dv_audio_dv_video_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        RemoteReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver aRx(aConfig, rConfig);
        aRx.init();

        VideoReceiverConfig vConfig("xvimagesink");
        RemoteReceiverConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver vRx(vConfig, rvConfig);
        vRx.init();

        BLOCK();

        TEST_ASSERT(aRx.stop());
        TEST_ASSERT(vRx.stop());

        TEST_ASSERT(!aRx.isPlaying());
        TEST_ASSERT(!vRx.isPlaying());
    }
    else {
        AudioLocalConfig aConfig("dv1394src", numChannels);
        RemoteSenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender aTx(aConfig, rConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src");
        RemoteSenderConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoSender vTx(vConfig, rvConfig);
        vTx.init();

        BLOCK();

        TEST_ASSERT(aTx.stop());
        TEST_ASSERT(vTx.stop());

        TEST_ASSERT(!aTx.isPlaying());
        TEST_ASSERT(!vTx.isPlaying());
    }
}


void SyncTestSuite::start_stop_dv_audio_dv_video_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        RemoteReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver aRx(aConfig, rConfig);
        aRx.init();

        VideoReceiverConfig vConfig("xvimagesink");
        RemoteReceiverConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver vRx(vConfig, rvConfig);
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
    else {
        AudioLocalConfig aConfig("dv1394src", numChannels);
        RemoteSenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender aTx(aConfig, rConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src");
        RemoteSenderConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoSender vTx(vConfig, rvConfig);
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
        AudioReceiverConfig aConfig("jackaudiosink");
        RemoteReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver aRx(aConfig, rConfig);
        aRx.init();
        
        VideoReceiverConfig vConfig("xvimagesink");
        RemoteReceiverConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoReceiver vRx(vConfig, rvConfig);
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
    else {
        AudioLocalConfig aConfig("audiotestsrc", NUM_CHANNELS);
        RemoteSenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender aTx(aConfig, rConfig);
        aTx.init();

        VideoConfig vConfig("dv1394src");
        RemoteSenderConfig rvConfig("h264", get_host_ip(), V_PORT);
        VideoSender vTx(vConfig, rvConfig);
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
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "audioTester <0/1>" << std::endl;
        return 1;
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    SyncTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


