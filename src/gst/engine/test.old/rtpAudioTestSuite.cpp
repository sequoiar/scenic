/* audioTestSuite.cpp
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
#include "rtpAudioTestSuite.h"
#include "audioSender.h"
#include "audioReceiver.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "codec.h"
#include "playback.h"

#include "gst/audioFactory.h"

#include "tcp/singleBuffer.h"

/*----------------------------------------------*/ 
/* Helper functions                             */
/*----------------------------------------------*/ 

/*----------------------------------------------*/ 
/* Unit tests.                                  */
/*----------------------------------------------*/ 


void RtpAudioTestSuite::start_stop_2ch_audiotest()
{
    const int NUM_CHANNELS = 2;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver());

        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("audiotestsrc", NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));
        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}



void RtpAudioTestSuite::start_stop_8ch_audiotest()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("audiotestsrc", NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}



void RtpAudioTestSuite::start_stop_6ch_alsa()
{
    const int NUM_CHANNELS = 6;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver("alsasink"));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("alsasrc", NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}



void RtpAudioTestSuite::start_stop_6ch_pulse()
{
    const int NUM_CHANNELS = 6;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver("pulsesink"));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("pulsesrc", NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}




void RtpAudioTestSuite::start_stop_8ch_jack()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver(ports::IP, "vorbis"));

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("jackaudiosrc", NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig, ports::IP, "vorbis"));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}



void RtpAudioTestSuite::start_stop_8ch_jack_vorbis()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("jackaudiosrc", NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}



void RtpAudioTestSuite::start_stop_8ch_audiofile()
{
    const int NUM_CHANNELS = 8;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver());

        playback::start();

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("filesrc", audioFilename_, NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_audio_dv()
{
    const int NUM_CHANNELS = 2;
    if (id_ == 0) {
        boost::shared_ptr<AudioReceiver> rx(audiofactory::buildAudioReceiver());

        playback::start();

        BLOCK();

        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
    }
    else {
        AudioSourceConfig aConfig("dv1394src", NUM_CHANNELS);
        boost::shared_ptr<AudioSender> tx(audiofactory::buildAudioSender(aConfig));

        playback::start();

        TEST_ASSERT(tcpSendBuffer(ports::IP, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, tx->getCaps()));

        BLOCK();
        TEST_ASSERT(playback::isPlaying());

        playback::stop();
        TEST_ASSERT(!playback::isPlaying());
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

