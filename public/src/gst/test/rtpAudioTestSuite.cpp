
// audioTestSuite.cpp
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
#include "hostIP.h"
#include "rtpAudioTestSuite.h"
//#define USE_OSC
#include "audioSender.h"
#include "audioReceiver.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "tcp/tcpThread.h"
#include "tcp/parser.h"

#include <sstream>

bool tcpGetCaps(int port,AudioReceiver &rx)
{
    TcpThread tcp(port);
        tcp.run();
        QueuePair& queue = tcp.getQueue();
        while(1)
        {
            MapMsg f = queue.timed_pop(100000);
            if(f["command"].type() == 'n')
                continue;

            GET_OR_RETURN(f,"command",std::string,command);
            GET_OR_RETURN(f,"caps_str",std::string,caps_str);
            rx.set_caps(caps_str.c_str());
            break;

        }


    return true;

}

void RtpAudioTestSuite::start_2ch_rtp_audiotest()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());
        
        TEST_ASSERT(tcpGetCaps(A_PORT+100,rx));

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        TEST_ASSERT(tx.start());

        MapMsg msg;

        std::ostringstream s;

        s  << "caps: caps_str=\"" << strEsq(tx.getCaps()) <<"\"" << std::endl;
        tokenize(s.str(),msg);

        TcpThread tcp(A_PORT+100);
        tcp.socket_connect_send("127.0.0.1",msg);


        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_2ch_rtp_audiotest()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_2ch_rtp_audiotest()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        TEST_ASSERT(rx.start());

        BLOCK();

        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_8ch_rtp_audiotest()
{
    const int numChannels = 8;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_8ch_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_8ch_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_8ch_rtp_audiofile()
{
    int numChannels = 8;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", fileLocation_, numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());


        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_8ch_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());


        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", fileLocation_, numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_8ch_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());



        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", fileLocation_, numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());


        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_audio_dv_rtp()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_audio_dv_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_audio_dv_rtp()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioReceiverConfig aConfig("jackaudiosink");
        ReceiverConfig rConfig("vorbis", get_host_ip(), A_PORT); 
        AudioReceiver rx(aConfig, rConfig);
        TEST_ASSERT(rx.init());

        TEST_ASSERT(rx.start());

        BLOCK();

        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", numChannels);
        SenderConfig rConfig("vorbis", get_host_ip(), A_PORT);
        AudioSender tx(aConfig, rConfig);
        TEST_ASSERT(tx.init());


        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


int main(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "rtpAudioTester <0/1>" << std::endl;
        return 1;
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    RtpAudioTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


