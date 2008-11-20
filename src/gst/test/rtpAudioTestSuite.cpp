
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
#include "audioSender.h"
#include "audioReceiver.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "ports.h"
#include "codec.h"

#include <sstream>


#include "capsHelper.h"

/*----------------------------------------------*/ 
/* Helper functions                             */
/*----------------------------------------------*/ 


std::auto_ptr<AudioSender> buildAudioSender(const AudioConfig aConfig)
{
    SenderConfig rConfig("vorbis", get_host_ip(), Ports::A_PORT);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    tx->init();
    return tx;
}


std::auto_ptr<AudioReceiver> buildAudioReceiver(const char *audioSinkName = "jackaudiosink")
{
    AudioSinkConfig aConfig(audioSinkName);
    ReceiverConfig rConfig("vorbis", get_host_ip(), Ports::A_PORT, tcpGetCaps(Ports::CAPS_PORT));
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}


std::auto_ptr<AudioReceiver> buildDeadAudioReceiver(const char *audioSinkName = "jackaudiosink")
{
    AudioSinkConfig aConfig(audioSinkName);
    ReceiverConfig rConfig("vorbis", get_host_ip(), Ports::A_PORT, "");
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}


/*----------------------------------------------*/ 
/* Unit tests.                                  */
/*----------------------------------------------*/ 


void RtpAudioTestSuite::start_2ch_audiotest()
{
    const int NUM_CHANNELS = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));
        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());
    }
}


void RtpAudioTestSuite::stop_2ch_audiotest()
{
    const int NUM_CHANNELS = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildDeadAudioReceiver());

        BLOCK();

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        BLOCK();

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_2ch_audiotest()
{
    const int NUM_CHANNELS = 2;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();

        TEST_ASSERT(rx->isPlaying());

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));
        BLOCK();
        TEST_ASSERT(tx->isPlaying());

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_8ch_audiotest()
{
    const int NUM_CHANNELS = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());
    }
}


void RtpAudioTestSuite::stop_8ch_audiotest()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildDeadAudioReceiver());

        BLOCK();

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        BLOCK();

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_8ch_audiotest()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("audiotestsrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_6ch_alsa()
{
    const int NUM_CHANNELS = 6;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver("alsasink"));

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());
    }
    else {
        AudioConfig aConfig("alsasrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());
    }
}


void RtpAudioTestSuite::stop_6ch_alsa()
{
    const int NUM_CHANNELS = 6;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildDeadAudioReceiver("alsasink"));

        BLOCK();

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("alsasrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        BLOCK();

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_6ch_alsa()
{
    const int NUM_CHANNELS = 6;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver("alsasink"));

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("alsasrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_6ch_pulse()
{
    const int NUM_CHANNELS = 6;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver("pulsesink"));

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());
    }
    else {
        AudioConfig aConfig("pulsesrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());
    }
}


void RtpAudioTestSuite::stop_6ch_pulse()
{
    const int NUM_CHANNELS = 6;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildDeadAudioReceiver("pulsesink"));

        BLOCK();

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("pulsesrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        BLOCK();

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_6ch_pulse()
{
    const int NUM_CHANNELS = 6;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver("pulsesink"));

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("pulsesrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_8ch_jack()
{
    const int NUM_CHANNELS = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();
        //FIXME: figure out how to set layout
#if 0
        rx->getDecoder()->setSrcCaps();
        LOG_DEBUG("CAPS SET?");
#endif

        BLOCK();
        TEST_ASSERT(rx->isPlaying());
    }
    else {
        AudioConfig aConfig("jackaudiosrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());
    }
}


void RtpAudioTestSuite::stop_8ch_jack()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildDeadAudioReceiver());

        BLOCK();

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("jackaudiosrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        BLOCK();

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_8ch_jack()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("jackaudiosrc", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_8ch_audiofile()
{
    const int NUM_CHANNELS = 8;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", audioFilename_, NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());
    }
}


void RtpAudioTestSuite::stop_8ch_audiofile()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildDeadAudioReceiver());

        BLOCK();

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", audioFilename_, NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        BLOCK();

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_8ch_audiofile()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("filesrc", audioFilename_, NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_audio_dv()
{
    const int NUM_CHANNELS = 2;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();
        TEST_ASSERT(rx->isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());
    }
}


void RtpAudioTestSuite::stop_audio_dv()
{
    const int NUM_CHANNELS = 2;

    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildDeadAudioReceiver());

        BLOCK();

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        BLOCK();

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_audio_dv()
{
    const int NUM_CHANNELS = 2;
    if (id_ == 0) {
        std::auto_ptr<AudioReceiver> rx(buildAudioReceiver());

        rx->start();

        BLOCK();

        TEST_ASSERT(rx->isPlaying());

        rx->stop();
        TEST_ASSERT(!rx->isPlaying());
    }
    else {
        AudioConfig aConfig("dv1394src", NUM_CHANNELS);
        std::auto_ptr<AudioSender> tx(buildAudioSender(aConfig));

        tx->start();

        TEST_ASSERT(tcpSendCaps("127.0.0.1", Ports::CAPS_PORT, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(tx->isPlaying());

        tx->stop();
        TEST_ASSERT(!tx->isPlaying());
    }
}


int mainRtpAudioTestSuite(int argc, char **argv)
{
    if (!GstTestSuite::areValidArgs(argc, argv)) {
        std::cerr << "Usage: " << "rtpAudioTester <0/1>" << std::endl;
        return 1;
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    RtpAudioTestSuite tester;
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

