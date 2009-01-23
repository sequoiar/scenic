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
#include "audioTestSuite.h"
#include "audioLocal.h"
#include "audioConfig.h"
#include "mapMsg.h"
#include "playback.h"

class GstAudioTestSubscriber : public msg::Subscriber
{
    std::string s_;
    public:
    GstAudioTestSubscriber(std::string s):s_(s+"*"+__FUNCTION__) {}
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


void AudioTestSuite::start_stop_1ch_audiotest()
{
    const int NUM_CHANNELS = 1;
    AudioSourceConfig srcConfig("audiotestsrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("jackaudiosink");
    AudioLocal tx(srcConfig, sinkConfig);
    GstAudioTestSubscriber f(__FUNCTION__);  // Grabs the MSG::post callback, used by AudioLevel
    TEST_THROWS_NOTHING(tx.init());
    

    playback::start();

    BLOCK();

    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}


void AudioTestSuite::start_stop_2ch_audiotest()
{
    const int NUM_CHANNELS = 2;
    AudioSourceConfig srcConfig("audiotestsrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("jackaudiosink");
    AudioLocal tx(srcConfig, sinkConfig);
    GstAudioTestSubscriber f(__FUNCTION__);  // Grabs the MSG::post callback 
    TEST_THROWS_NOTHING(tx.init());
    
    playback::start();

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_6ch_audiotest()
{
    const int NUM_CHANNELS = 6;
    AudioSourceConfig srcConfig("audiotestsrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("jackaudiosink");
    AudioLocal tx(srcConfig, sinkConfig);
    GstAudioTestSubscriber f(__FUNCTION__);  // Grabs the MSG::post callback 
    TEST_THROWS_NOTHING(tx.init());
    

    playback::start();

    BLOCK();

    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_8ch_audiotest()
{
    const int NUM_CHANNELS = 8;
    AudioSourceConfig srcConfig("audiotestsrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("jackaudiosink");
    AudioLocal tx(srcConfig, sinkConfig);
    TEST_THROWS_NOTHING(tx.init());
    

    playback::start();

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_8ch_jack()
{
    const int NUM_CHANNELS = 8;
    AudioSourceConfig srcConfig("jackaudiosrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("jackaudiosink");
    AudioLocal tx(srcConfig, sinkConfig);
    TEST_THROWS_NOTHING(tx.init());
    playback::start();

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_8ch_audiofile()
{
    const int NUM_CHANNELS = 8;

    AudioSourceConfig srcConfig("filesrc", audioFilename_, NUM_CHANNELS);
    AudioSinkConfig sinkConfig("jackaudiosink");
    AudioLocal tx(srcConfig, sinkConfig);
    TEST_THROWS_NOTHING(tx.init());

    TEST_THROWS_NOTHING(playback::start());

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    TEST_THROWS_NOTHING(playback::stop());
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_audio_dv()
{
    const int NUM_CHANNELS = 2;
    AudioSourceConfig srcConfig("dv1394src", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("jackaudiosink");
    AudioLocal tx(srcConfig, sinkConfig);
    TEST_THROWS_NOTHING(tx.init());

    playback::start();

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_8ch_alsa()
{
    const int NUM_CHANNELS = 8;
    AudioSourceConfig srcConfig("alsasrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("alsasink");
    AudioLocal tx(srcConfig, sinkConfig);
    TEST_THROWS_NOTHING(tx.init());
    playback::start();

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_6ch_pulse()
{
    const int NUM_CHANNELS = 6;
    AudioSourceConfig srcConfig("pulsesrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("pulsesink");
    AudioLocal tx(srcConfig, sinkConfig);
    TEST_THROWS_NOTHING(tx.init());
    playback::start();

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}



void AudioTestSuite::start_stop_8ch_pulse()
{
    const int NUM_CHANNELS = 8;
    AudioSourceConfig srcConfig("pulsesrc", NUM_CHANNELS);
    AudioSinkConfig sinkConfig("pulsesink");
    AudioLocal tx(srcConfig, sinkConfig);
    TEST_THROWS_NOTHING(tx.init());
    playback::start();

    BLOCK();
    TEST_ASSERT(playback::isPlaying());

    playback::stop();
    TEST_ASSERT(!playback::isPlaying());
}


int mainAudioTestSuite(int /*argc*/, char ** /*argv*/)
{
    std::cout << "Built on " << __DATE__ << " at " << __TIME__ << std::endl;
    AudioTestSuite tester;

    Test::TextOutput output(Test::TextOutput::Verbose);
    return tester.run(output) ? EXIT_SUCCESS : EXIT_FAILURE;
}


