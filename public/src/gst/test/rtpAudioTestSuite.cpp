
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


void RtpAudioTestSuite::start_2ch_rtp_audiotest()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbis", get_host_ip(), A_PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_2ch_rtp_audiotest()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_2ch_rtp_audiotest()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();

        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

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
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_8ch_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_8ch_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
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
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", fileLocation_, numChannels, "vorbis",
                           get_host_ip(), A_PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_8ch_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", fileLocation_, numChannels, "vorbis",
                           get_host_ip(), A_PORT);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_8ch_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", fileLocation_, numChannels, "vorbis",
                           get_host_ip(), A_PORT);
        AudioSender tx(config);
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
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("dv1394src", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void RtpAudioTestSuite::stop_audio_dv_rtp()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("dv1394src", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void RtpAudioTestSuite::start_stop_audio_dv_rtp()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbis", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();

        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("dv1394src", numChannels, "vorbis", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

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


