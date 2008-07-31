
// gstTestSuite.cpp
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

#include "gstTestSuite2.h"
#include "videoSender.h"
#include "videoConfig.h"
#include "videoReceiver.h"
#include "audioSender.h"
#include "audioReceiver.h"
#include "audioConfig.h"

#include "hostIP.h"

void GstTestSuite::set_id(int id)
{
    if (id == 1 || id == 0)
        id_ = id;
    else {
        std::cerr << "Id must be 0 or 1." << std::endl;
        exit(1);
    }
}


void GstTestSuite::setup()
{
    std::cout.flush();
    std::cout << std::endl;
}


void GstTestSuite::tear_down()
{
    // empty
}


void GstTestSuite::init_test()
{
    if (id_ == 1)
        return;
    VideoConfig config("videotestsrc");
    VideoSender tx(config);
    tx.init();

    BLOCK();
}


void GstTestSuite::start_test_video()
{
    if (id_ == 1)
        return;
    VideoConfig config("videotestsrc");
    VideoSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());
}


void GstTestSuite::stop_test_video()
{
    if (id_ == 1)
        return;
    VideoConfig config("videotestsrc");
    VideoSender tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_stop_test_video()
{
    if (id_ == 1)
        return;
    VideoConfig config("videotestsrc");
    VideoSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_v4l()
{
    if (id_ == 1)
        return;
    VideoConfig config("v4l2src");
    VideoSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void GstTestSuite::stop_v4l()
{
    if (id_ == 1)
        return;
    VideoConfig config("v4l2src");
    VideoSender tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_stop_v4l()
{
    if (id_ == 1)
        return;
    VideoConfig config("v4l2src");
    VideoSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_v4l_rtp()
{
    const int PORT = 10010;

    if (id_ == 0) {
        VideoConfig config("h264", PORT);
        VideoReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());


/*        GstQuery *query;
        gboolean res = false;
        GstElement *e = rx.getElement();
        query = gst_query_new_latency();
        res = gst_element_query(e,query);
        while(1) {
            GstClockTime min,max;
            gst_query_parse_latency(query,NULL,&min,&max);
               std::cout << min << "min";
               std::cout << max << "max" << std::endl;
        }
 */
        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        VideoConfig config("videotestsrc", "h264", get_host_ip(), PORT);
        VideoSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void GstTestSuite::stop_v4l_rtp()
{
    const int PORT = 10010;
    if (id_ == 0) {
        VideoConfig config("h264", PORT);
        VideoReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig config("videotestsrc", "h264", get_host_ip(), PORT);
        VideoSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_stop_v4l_rtp()
{
    const int PORT = 10010;
    if (id_ == 0) {
        VideoConfig config("h264", PORT);
        VideoReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig config("videotestsrc", "h264", get_host_ip(), PORT);
        VideoSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_dv()
{
    if (id_ == 1)
        return;
    VideoConfig config("dv1394src");
    VideoSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void GstTestSuite::stop_dv()
{
    if (id_ == 1)
        return;
    VideoConfig config("dv1394src");
    VideoSender tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_stop_dv()
{
    if (id_ == 1)
        return;
    VideoConfig config("dv1394src");
    VideoSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_dv_rtp()
{
    const int PORT = 10010;
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0) {
        VideoConfig config("h264", PORT);
        VideoReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        VideoConfig config("dv1394src", "h264", get_host_ip(), PORT);
        VideoSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void GstTestSuite::stop_dv_rtp()
{
    const int PORT = 10010;
    if (id_ == 0) {
        VideoConfig config("h264", PORT);
        VideoReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig config("dv1394src", "h264", get_host_ip(), PORT);
        VideoSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_stop_dv_rtp()
{
    const int PORT = 10010;
    if (id_ == 0) {
        VideoConfig config("h264", PORT);
        VideoReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        VideoConfig config("dv1394src", "h264", get_host_ip(), PORT);
        VideoSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("filesrc.delay", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void GstTestSuite::stop_1ch_audiotest()
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


void GstTestSuite::start_stop_1ch_audiotest()
{
    if (id_ == 1)
        return;
    int numChannels = 1;
    AudioConfig config("audiotestsrc.delay", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_2ch_audiotest()
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


void GstTestSuite::stop_2ch_audiotest()
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


void GstTestSuite::start_stop_2ch_audiotest()
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


void GstTestSuite::start_6ch_audiotest()
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


void GstTestSuite::stop_6ch_audiotest()
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


void GstTestSuite::start_stop_6ch_audiotest()
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


void GstTestSuite::start_8ch_audiotest()
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


void GstTestSuite::stop_8ch_audiotest()
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


void GstTestSuite::start_stop_8ch_audiotest()
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


void GstTestSuite::start_2ch_comp_rtp_audiotest()
{
    int numChannels = 2;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", 10010);
        AudioReceiver rx(config);
        rx.init();
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           10010);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void GstTestSuite::stop_2ch_comp_rtp_audiotest()
{
    int numChannels = 2;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", 10010);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           10010);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_stop_2ch_comp_rtp_audiotest()
{
    int numChannels = 2;
    const int PORT = 10010;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", PORT);
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
                           PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_8ch_comp_rtp_audiotest()
{
    const int PORT = 10010;
    const int numChannels = 8;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", PORT);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           PORT);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void GstTestSuite::stop_8ch_comp_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", 10010);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                           10010);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_stop_8ch_comp_rtp_audiotest()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", 10010);
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
                           10010);
        AudioSender tx(config);
        TEST_ASSERT(tx.init());

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_8ch_alsa()
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


void GstTestSuite::stop_8ch_alsa()
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


void GstTestSuite::start_stop_8ch_alsa()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("alsasrc.delay", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_8ch_jack()
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


void GstTestSuite::stop_8ch_jack()
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


void GstTestSuite::start_stop_8ch_jack()
{
    if (id_ == 1)
        return;
    int numChannels = 8;
    AudioConfig config("jackaudiosrc.delay", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_8ch_comp_audiofile()
{
    int numChannels = 8;

    if (id_ == 1)
        return;
    AudioConfig config("filesrc", numChannels);
    AudioSender tx(config);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void GstTestSuite::stop_8ch_comp_audiofile()
{
    int numChannels = 8;
    if (id_ == 1)
        return;
    AudioConfig config("filesrc", numChannels);
    AudioSender tx(config);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_stop_8ch_comp_audiofile()
{
    int numChannels = 8;
    if (id_ == 1)
        return;
    AudioConfig config("filesrc.delay", numChannels);
    AudioSender tx(config);
    TEST_ASSERT(tx.init());

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void GstTestSuite::start_8ch_comp_rtp_audiofile()
{
    int numChannels = 8;

    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", 10010);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", numChannels, "vorbisenc", get_host_ip(), 10010);
        AudioSender tx(config);
        tx.init();

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}


void GstTestSuite::stop_8ch_comp_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", 10010);
        AudioReceiver rx(config);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", numChannels, "vorbisenc", get_host_ip(), 10010);
        AudioSender tx(config);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_stop_8ch_comp_rtp_audiofile()
{
    int numChannels = 8;
    if (id_ == 0) {
        AudioConfig config(numChannels, "vorbisdec", 10010);
        AudioReceiver rx(config);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else {
        AudioConfig config("filesrc", numChannels, "vorbisenc", get_host_ip(), 10010);
        AudioSender tx(config);
        TEST_ASSERT(tx.init());

        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}


void GstTestSuite::start_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;
    int vPort = 10010;
    int aPort = vPort + 1000;

    if (id_ == 0) {
        VideoConfig vConfig("h264", vPort);
        VideoReceiver vRx(vConfig);
        vRx.init();

        AudioConfig aConfig(numChannels, "vorbisdec", aPort);
        AudioReceiver aRx(aConfig);
        aRx.init();

        TEST_ASSERT(vRx.start());
        TEST_ASSERT(aRx.start());

        BLOCK();
        TEST_ASSERT(vRx.isPlaying());
        TEST_ASSERT(aRx.isPlaying());
    }
    else {
        VideoConfig vConfig("dv1394src", "h264", get_host_ip(), vPort);
        VideoSender vTx(vConfig);
        vTx.init();

        AudioConfig aConfig("filesrc", numChannels, "vorbisenc", get_host_ip(), aPort);
        AudioSender aTx(aConfig);
        aTx.init();

        TEST_ASSERT(aTx.start());
        TEST_ASSERT(vTx.start());

        BLOCK();
        TEST_ASSERT(vTx.isPlaying());
        TEST_ASSERT(aTx.isPlaying());
    }
}


void GstTestSuite::stop_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;
    int vPort = 10010;
    int aPort = vPort + 1000;

    if (id_ == 0) {
        VideoConfig vConfig("h264", vPort);
        VideoReceiver vRx(vConfig);
        vRx.init();

        AudioConfig aConfig(numChannels, "vorbisdec", aPort);
        AudioReceiver aRx(aConfig);
        aRx.init();

        BLOCK();

        TEST_ASSERT(vRx.stop());
        TEST_ASSERT(aRx.stop());

        TEST_ASSERT(!vRx.isPlaying());
        TEST_ASSERT(!aRx.isPlaying());
    }
    else {
        VideoConfig vConfig("dv1394src", "h264", get_host_ip(), vPort);
        VideoSender vTx(vConfig);
        vTx.init();

        AudioConfig aConfig("filesrc", numChannels, "vorbisenc", get_host_ip(), aPort);
        AudioSender aTx(aConfig);
        aTx.init();

        BLOCK();
        TEST_ASSERT(vTx.stop());
        TEST_ASSERT(aTx.stop());

        TEST_ASSERT(!vTx.isPlaying());
        TEST_ASSERT(!aTx.isPlaying());
    }
}


void GstTestSuite::start_stop_8ch_comp_rtp_audiofile_dv()
{
    int numChannels = 8;
    int vPort = 10010;
    int aPort = vPort + 1000;

    if (id_ == 0) {
        VideoConfig vConfig("h264", vPort);
        VideoReceiver vRx(vConfig);
        vRx.init();

        AudioConfig aConfig(numChannels, "vorbisdec", aPort);
        AudioReceiver aRx(aConfig);
        aRx.init();

        TEST_ASSERT(vRx.start());
        TEST_ASSERT(aRx.start());

        BLOCK();

        TEST_ASSERT(vRx.isPlaying());
        TEST_ASSERT(aRx.isPlaying());

        TEST_ASSERT(vRx.stop());
        TEST_ASSERT(aRx.stop());

        TEST_ASSERT(!vRx.isPlaying());
        TEST_ASSERT(!aRx.isPlaying());
    }
    else {
        VideoConfig vConfig("dv1394src", "h264", get_host_ip(), vPort);
        VideoSender vTx(vConfig);
        vTx.init();

        AudioConfig aConfig("filesrc", numChannels, "vorbisenc", get_host_ip(), aPort);
        AudioSender aTx(aConfig);
        aTx.init();

        TEST_ASSERT(vTx.start());
        TEST_ASSERT(aTx.start());

        BLOCK();

        TEST_ASSERT(vTx.isPlaying());
        TEST_ASSERT(aTx.isPlaying());

        TEST_ASSERT(vTx.stop());
        TEST_ASSERT(aTx.stop());

        TEST_ASSERT(!vTx.isPlaying());
        TEST_ASSERT(!aTx.isPlaying());
    }
}


#if 0
void GstTestSuite::sync()
{
    int numChannels = 8;
    int vPort = 10010;
    int aPort = vPort + 1000;

    if (id_ == 0) {
        VideoConfig vConfig("h264", vPort);
        VideoReceiver vRx(vConfig);
        vRx.init();

        AudioConfig aConfig(numChannels, "vorbisdec", aPort);
        AudioReceiver aRx(aConfig);
        aRx.init();

        TEST_ASSERT(vRx.start());
        TEST_ASSERT(aRx.start());

        BLOCK();

        TEST_ASSERT(vRx.isPlaying());
        TEST_ASSERT(aRx.isPlaying());

        TEST_ASSERT(vRx.stop());
        TEST_ASSERT(aRx.stop());

        TEST_ASSERT(!vRx.isPlaying());
        TEST_ASSERT(!aRx.isPlaying());
    }
    else {
        VideoConfig vConfig("videotestsrc", "h264", get_host_ip(), vPort);
        VideoSender vTx(vConfig);
        vTx.init();

        AudioConfig aConfig("audiotestsrc", numChannels, "vorbisenc", get_host_ip(),
                            aPort);
        AudioSender aTx(aConfig);
        aTx.init();

        TEST_ASSERT(vTx.start());
        TEST_ASSERT(aTx.start());

        BLOCK();

        TEST_ASSERT(vTx.isPlaying());
        TEST_ASSERT(aTx.isPlaying());

        TEST_ASSERT(vTx.stop());
        TEST_ASSERT(aTx.stop());

        TEST_ASSERT(!vTx.isPlaying());
        TEST_ASSERT(!aTx.isPlaying());
    }
}


#endif


int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << "gstTester <0/1>" << std::endl;
        exit(1);
    }
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    GstTestSuite tester;
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


