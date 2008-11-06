
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
#include "audioTestSuite.h"
#include "audioLocal.h"
#include "audioConfig.h"
#include "mapMsg.h"

class TestMsgFunctor : public MsgFunctor
{
    std::string s_;
    public:
    TestMsgFunctor(std::string s):s_(s+"*"+__FUNCTION__) { MSG::register_cb(this); }
    ~TestMsgFunctor() { MSG::unregister_cb();}
    void operator()(MapMsg& msg) 
    {
        std::vector<double> v_ = msg["values"];
        std::vector<double>::const_iterator it;
        std::stringstream st;
        st << "Msg: " << msg["command"].c_str() << " received by " << s_; 
        for(it = v_.begin();it != v_.end();++it)
            st << " " << *it;

        LOG_DEBUG(st.str()) ;
    } 
};

void AudioTestSuite::start_1ch_audiotest()
{
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_1ch_audiotest()
{
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    tx.stop();

    BLOCK();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_1ch_audiotest()
{
    int numChannels = 1;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TestMsgFunctor f(__FUNCTION__);  // Grabs the MSG::post callback 
    TEST_THROWS_NOTHING(tx.init());
    

    tx.start();

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_2ch_audiotest()
{
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_2ch_audiotest()
{
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    

    BLOCK();

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_2ch_audiotest()
{
    int numChannels = 2;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TestMsgFunctor f(__FUNCTION__);  // Grabs the MSG::post callback 
    TEST_THROWS_NOTHING(tx.init());
    

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_6ch_audiotest()
{
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_6ch_audiotest()
{
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    

    BLOCK();

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_6ch_audiotest()
{
    int numChannels = 6;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TestMsgFunctor f(__FUNCTION__);  // Grabs the MSG::post callback 
    TEST_THROWS_NOTHING(tx.init());
    

    tx.start();

    BLOCK();

    TEST_ASSERT(tx.isPlaying());

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_audiotest()
{
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_audiotest()
{
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    

    BLOCK();
    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_audiotest()
{
    int numChannels = 8;
    AudioConfig config("audiotestsrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_jack()
{
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_jack()
{
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    BLOCK();
    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_jack()
{
    int numChannels = 8;
    AudioConfig config("jackaudiosrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_audiofile()
{
    int numChannels = 8;
    const int LOOP_COUNT = 4;

    AudioConfig config("filesrc", audioFilename_, numChannels, LOOP_COUNT);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    TEST_THROWS_NOTHING(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_audiofile()
{
    int numChannels = 8;
    AudioConfig config("filesrc", audioFilename_, numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    BLOCK();

    TEST_THROWS_NOTHING(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_audiofile()
{
    int numChannels = 8;
    AudioConfig config("filesrc", audioFilename_, numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    TEST_THROWS_NOTHING(tx.start());

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    TEST_THROWS_NOTHING(tx.stop());
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_audio_dv()
{
    int numChannels = 2;
    AudioConfig config("dv1394src", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_audio_dv()
{
    int numChannels = 2;
    AudioConfig config("dv1394src", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    TEST_ASSERT(!tx.isPlaying());

    BLOCK();
    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_audio_dv()
{
    int numChannels = 2;
    AudioConfig config("dv1394src", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_8ch_alsa()
{
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());
}


void AudioTestSuite::stop_8ch_alsa()
{
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());

    BLOCK();
    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


void AudioTestSuite::start_stop_8ch_alsa()
{
    int numChannels = 8;
    AudioConfig config("alsasrc", numChannels);
    AudioLocal tx(config);
    TEST_THROWS_NOTHING(tx.init());
    tx.start();

    BLOCK();
    TEST_ASSERT(tx.isPlaying());

    tx.stop();
    TEST_ASSERT(!tx.isPlaying());
}


int mainAudioTestSuite(int /*argc*/, char ** /*argv*/)
{
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    AudioTestSuite tester;

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


