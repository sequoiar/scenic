
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

#include "hostIP.h"
#include "audioTestSuite.h"
#include "audioSender.h"
#include "audioReceiver.h"
#include "audioConfig.h"

void AudioTestSuite::start_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();
    TEST_ASSERT(tx.stop());

    BLOCK();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_2ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_2ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_2ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_6ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_6ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_6ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_2ch_comp_rtp_audiotest()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void AudioTestSuite::stop_2ch_comp_rtp_audiotest()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void AudioTestSuite::start_stop_2ch_comp_rtp_audiotest()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();

        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
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


void AudioTestSuite::start_8ch_comp_rtp_audiotest()
{
    const int numChannels = 8;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void AudioTestSuite::stop_8ch_comp_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           A_PORT);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void AudioTestSuite::start_stop_8ch_comp_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
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


void AudioTestSuite::start_8ch_alsa()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_alsa()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_alsa()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_jack()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_jack()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_jack()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_comp_audiofile()
{
    int numChannels = 8;

    if (id_ == 1)
        return;
    AudioConfig config("filesrc", "test_signal8.wav", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_comp_audiofile()
{
    int numChannels = 8;
    if (id_ == 1)
        return;
    AudioConfig config("filesrc", "test_signal8.wav", numChannels);
    AudioSender tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_comp_audiofile()
{
    int numChannels = 8;
    if (id_ == 1)
        return;
    AudioConfig config("filesrc", "test_signal8.wav", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_comp_rtp_audiofile()
{
    int numChannels = 8;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", "test_signal8.wav", numChannels, "vorbisenc",
                           get_host_ip(), A_PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void AudioTestSuite::stop_8ch_comp_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", "test_signal8.wav", numChannels, "vorbisenc",
                           get_host_ip(), A_PORT);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void AudioTestSuite::start_stop_8ch_comp_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", A_PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", "test_signal8.wav", numChannels, "vorbisenc",
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


void AudioTestSuite::start_audio_dv()
{
    int numChannels = 2;
    if (id_ == 1)
        return;
    AudioConfig config("dv1394src", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_audio_dv()
{
    int numChannels = 2;
    if (id_ == 1)
        return;
    AudioConfig config("dv1394src", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(!tx.isPlaying());

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_audio_dv()
{
    int numChannels = 2;
    if (id_ == 1)
        return;
    AudioConfig config("dv1394src", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << "audioTester <0/1>" << std::endl;
        exit(1);
    }
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    AudioTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


