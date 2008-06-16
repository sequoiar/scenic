
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

#include "gstTestSuite.h"
#include "videoSender.h"
#include "videoSession.h"
#include "videoReceiver.h"
#include "audioSender.h"
#include "audioReceiver.h"
#include "audioSession.h"

/*----------------------------------------------*/ 
// To actually observe the tests (watch/listen), set
// this macro to 1.
/*----------------------------------------------*/ 

void GstTestSuite::set_id(int id)
{
    if (id == 1 || id == 0)
        id_ = id;
    else 
    {
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

    VideoSession session("videotestsrc");
    VideoSender tx(session);
    tx.init();

    BLOCK();
}



void GstTestSuite::start_test_video()
{
    if (id_ == 1)
        return;

    VideoSession session("videotestsrc");
    VideoSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());
}



void GstTestSuite::stop_test_video()
{
    if (id_ == 1)
        return;
    
    VideoSession session("videotestsrc");
    VideoSender tx(session);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_stop_test_video()
{
    if (id_ == 1)
        return;

    VideoSession session("videotestsrc");
    VideoSender tx(session);
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

    VideoSession session("v4l2src");
    VideoSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}



void GstTestSuite::stop_v4l()
{
    if (id_ == 1)
        return;

    VideoSession session("v4l2src");
    VideoSender tx(session);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_stop_v4l()
{
    if (id_ == 1)
        return;

    VideoSession session("v4l2src");
    VideoSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_v4l_rtp()
{
    if (id_ == 0)
    {
        VideoSession session("v4l2src", "h264", MY_ADDRESS, 10010);
        VideoSender tx(session);
        tx.init();
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
    else
    {
        VideoSession session(10010);
        VideoReceiver rx(session);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
}



void GstTestSuite::stop_v4l_rtp()
{
    if (id_ == 0)
    {
        VideoSession session("v4l2src", "h264", MY_ADDRESS, 10010);
        VideoSender tx(session);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
    else
    {
        VideoSession session(10010);
        VideoReceiver rx(session);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
}



void GstTestSuite::start_stop_v4l_rtp()
{
    if (id_ == 0)
    {
        VideoSession session("v4l2src", "h264", MY_ADDRESS, 10010);
        VideoSender tx(session);
        tx.init();
        
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
    else
    {
        VideoSession session(10010);
        VideoReceiver rx(session);
        rx.init();
        
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
}



void GstTestSuite::start_dv()
{
    if (id_ == 1)
        return;
        
    VideoSession session("dv1394src");
    VideoSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}



void GstTestSuite::stop_dv()
{
    if (id_ == 1)
        return;

    VideoSession session("dv1394src");
    VideoSender tx(session);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_stop_dv()
{
    if (id_ == 1)
        return;

    VideoSession session("dv1394src");
    VideoSender tx(session);
    tx.init();
    
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_dv_rtp()
{
    // receiver should be started first, of course there's no guarantee that it will at this point
    if (id_ == 0)
    {
        VideoSession session(10010);
        VideoReceiver rx(session);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else
    {
        VideoSession session("dv1394src", "h264", MY_ADDRESS, 10010);
        VideoSender tx(session);
        tx.init();
        
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}



void GstTestSuite::stop_dv_rtp()
{
    if (id_ == 0)
    {
        VideoSession session(10010);
        VideoReceiver rx(session);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        VideoSession session("dv1394src", "h264", MY_ADDRESS, 10010);
        VideoSender tx(session);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void GstTestSuite::start_stop_dv_rtp()
{
    if (id_ == 0)
    {
        VideoSession session(10010);
        VideoReceiver rx(session);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        VideoSession session("dv1394src", "h264", MY_ADDRESS, 10010);
        VideoSender tx(session);
        tx.init();
        
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void GstTestSuite::start_1ch_audio()
{
    if (id_ == 1)
        return;

    int numChannels = 1;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}



void GstTestSuite::stop_1ch_audio()
{
    if (id_ == 1)
        return;

    int numChannels = 1;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();
    TEST_ASSERT(tx.stop());

    BLOCK();
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_stop_1ch_audio()
{
    if (id_ == 1)
        return;

    int numChannels = 1;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();
    
    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_2ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 2;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}



void GstTestSuite::stop_2ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 2;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_stop_2ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 2;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_6ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 6;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}



void GstTestSuite::stop_6ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 6;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    BLOCK();

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_stop_6ch_audio()
{
    if (id_ == 1)
        return;

    int numChannels = 6;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_8ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 8;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}



void GstTestSuite::stop_8ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 8;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();

    BLOCK();
    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_stop_8ch_audio()
{
    if (id_ == 1)
        return;
    
    int numChannels = 8;
    AudioSession session(numChannels);
    AudioSender tx(session);
    tx.init();
    
    TEST_ASSERT(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_ASSERT(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}



void GstTestSuite::start_2ch_comp_rtp_audio()
{
    int numChannels = 2;
    if (id_ == 0)
    {
        AudioSession session(numChannels, "vorbis", 10010);
        AudioReceiver rx(session);
        rx.init();
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else
    {
        AudioSession session(numChannels, "vorbis", MY_ADDRESS, 10010);
        AudioSender tx(session);
        tx.init();
    
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}



void GstTestSuite::stop_2ch_comp_rtp_audio()
{
    int numChannels = 2;

    if (id_ == 0)
    {
        AudioSession session(numChannels, "vorbis", 10010);
        AudioReceiver rx(session);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        AudioSession session(numChannels, "vorbis", MY_ADDRESS, 10010);
        AudioSender tx(session);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void GstTestSuite::start_stop_2ch_comp_rtp_audio()
{
    int numChannels = 2;
    if (id_ == 0)
    {
        AudioSession session(numChannels, "vorbis", 10010);
        AudioReceiver rx(session);
        rx.init();
        
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        AudioSession session(numChannels, "vorbis", MY_ADDRESS, 10010);
        AudioSender tx(session);
        tx.init();
        
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void GstTestSuite::start_8ch_comp_rtp_audio()
{
    int numChannels = 8;

    if (id_ == 0)
    {
        AudioSession session(numChannels, "vorbis", 10010);
        AudioReceiver rx(session);
        rx.init();
        
        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());
    }
    else
    {
        AudioSession session(numChannels, "vorbis", MY_ADDRESS, 10010);
        AudioSender tx(session);
        tx.init();
        
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());
    }
}



void GstTestSuite::stop_8ch_comp_rtp_audio()
{
    int numChannels = 8;
    if (id_ == 0)
    {
        AudioSession session(numChannels, "vorbis", 10010);
        AudioReceiver rx(session);
        rx.init();

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        AudioSession session(numChannels, "vorbis", MY_ADDRESS, 10010);
        AudioSender tx(session);
        tx.init();

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void GstTestSuite::start_stop_8ch_comp_rtp_audio()
{
    int numChannels = 8;
    if (id_ == 0)
    {
        AudioSession session(numChannels, "vorbis", 10010);
        AudioReceiver rx(session);
        rx.init();

        TEST_ASSERT(rx.start());

        BLOCK();
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        AudioSession session(numChannels, "vorbis", MY_ADDRESS, 10010);
        AudioSender tx(session);
        TEST_ASSERT(tx.init());
        
        TEST_ASSERT(tx.start());

        BLOCK();
        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());

    }
}

#if 0

void GstTestSuite::start_1ch_uncomp_rtp_audio()
{
    int numChannels = 1;
    int port = 5002;

    if (id_ == 0)
    {
        AudioReceiver rx;
        rx.init_uncomp(port, numChannels);
        TEST_ASSERT(rx.start());

        BLOCK();

        TEST_ASSERT(rx.isPlaying());
    }
    else
    {
        AudioSender tx;
        tx.init("1chUncompRtpTest", port, MY_ADDRESS);
        TEST_ASSERT(tx.start());

        BLOCK();

        TEST_ASSERT(tx.isPlaying());
    }
}



void GstTestSuite::stop_1ch_uncomp_rtp_audio()
{
    int numChannels = 1;
    int port = 5002;

    if (id_ == 0)
    {
        AudioReceiver rx;
        rx.init_uncomp(port, numChannels);

        BLOCK();

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        AudioSender tx;
        tx.init("1chUncompRtpTest", port, MY_ADDRESS);

        BLOCK();

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void GstTestSuite::start_stop_1ch_uncomp_rtp_audio()
{
    int numChannels = 1;
    int port = 5002;

    if (id_ == 0)
    {
        AudioReceiver rx;
        rx.init_uncomp(port, numChannels);
        TEST_ASSERT(rx.start());

        BLOCK();
        
        TEST_ASSERT(rx.isPlaying());

        TEST_ASSERT(rx.stop());
        TEST_ASSERT(!rx.isPlaying());
    }
    else
    {
        AudioSender tx;
        tx.init("1chUncompRtpTest", port, MY_ADDRESS);
        TEST_ASSERT(tx.start());

        BLOCK();

        TEST_ASSERT(tx.isPlaying());

        TEST_ASSERT(tx.stop());
        TEST_ASSERT(!tx.isPlaying());
    }
}



void start_8ch_uncomp_rtp_audio()
{
    // not yet implemented
}



void stop_8ch_uncomp_rtp_audio()
{
    // not yet implemented
}



void start_stop_8ch_uncomp_rtp_audio()
{
    // not yet implemented
}

#endif

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << "gstTester <0/1>" << std::endl;
        exit(1);
    }

    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    GstTestSuite tester; 
    tester.set_id(atoi(argv[1]));

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}

