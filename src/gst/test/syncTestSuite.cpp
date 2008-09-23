
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
#include "syncTestSuite.h"
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
#include "tcp/tcpThread.h"
#include "tcp/parser.h"
#include <sstream>

/*----------------------------------------------*/ 
/* Helper functions                             */
/*----------------------------------------------*/ 

bool tcpGetCaps(int port, AudioReceiver &rx)
{
    LOG_DEBUG("Waiting for caps");
    TcpThread tcp(port);
    tcp.run();
    QueuePair& queue = tcp.getQueue();
    bool gotCaps = false;
    while(!gotCaps)
    {
        MapMsg f = queue.timed_pop(100000);
        if(f["command"].type() == 'n')
            continue;
        try
        {
            GET(f, "command", std::string, command);
            GET(f, "caps_str", std::string, caps_str);
            rx.set_caps(caps_str.c_str());

            // send quit command to Receiver TcpThread to make 
            // threads join on function exit (i.e. TcpThread's destructor)
            MapMsg q;
            q["command"] = StrIntFloat("quit");
            queue.push(q);
            gotCaps = true;
        }
        catch(ErrorExcept)
        {
            return false;
        }
    }

    return true;
}


bool tcpSendCaps(int port, const std::string &caps)
{
    MapMsg msg;
    std::ostringstream s;

    TcpThread tcp(port);
    s  << "caps: caps_str=\"" << strEsq(caps) <<"\"" << std::endl;
    tokenize(s.str(),msg);

    return tcp.socket_connect_send("127.0.0.1", msg);
}


std::auto_ptr<AudioSender> buildAudioSender(const AudioConfig aConfig)
{
    SenderConfig rConfig("vorbis", get_host_ip(), GstTestSuite::A_PORT);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    assert(tx->init());
    return tx;
}


std::auto_ptr<AudioReceiver> buildAudioReceiver()
{
    AudioReceiverConfig aConfig("jackaudiosink");
    ReceiverConfig rConfig("vorbis", get_host_ip(), GstTestSuite::A_PORT); 
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    assert(rx->init());
    return rx;
}


std::auto_ptr<VideoReceiver> buildVideoReceiver()
{
        VideoReceiverConfig vConfig("xvimagesink");
        ReceiverConfig rConfig("h264", get_host_ip(), GstTestSuite::V_PORT);
        std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
        assert(rx->init());
        return rx;
}


std::auto_ptr<VideoSender> buildVideoSender(const VideoConfig vConfig)
{
        SenderConfig rConfig("h264", get_host_ip(), GstTestSuite::V_PORT);
        std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
        assert(tx->init());
        return tx;
}

/*----------------------------------------------*/ 
/* Unit tests                                   */
/*----------------------------------------------*/ 


void SyncTestSuite::start_8ch_comp_rtp_audiofile_dv()
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
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();
        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());
    }
}


void SyncTestSuite::stop_8ch_comp_rtp_audiofile_dv()
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


void SyncTestSuite::start_stop_8ch_comp_rtp_audiofile_dv()
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


void SyncTestSuite::start_dv_audio_dv_video()
{
    int numChannels = 2;

    if (id_ == 1)
        return;

    AudioConfig aConfig("dv1394src", numChannels);
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

    AudioConfig aConfig("dv1394src", numChannels);
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

    AudioConfig aConfig("dv1394src", numChannels);
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
        TEST_ASSERT(tcpSendCaps(A_PORT + 100, aTx->getCaps()));

        TEST_ASSERT(vTx->start());

        BLOCK();

        TEST_ASSERT(aTx->isPlaying());
        TEST_ASSERT(vTx->isPlaying());
    }
}


void SyncTestSuite::stop_dv_audio_dv_video_rtp()
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


void SyncTestSuite::start_stop_dv_audio_dv_video_rtp()
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
    try {
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    catch (Except e)
    {
        std::cerr << e.msg_;
        return 1;
    }
}

