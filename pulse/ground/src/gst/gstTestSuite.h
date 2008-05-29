// gstTestSutie.h

#ifndef _GST_TEST_SUITE_H_
#define _GST_TEST_SUITE_H_

#include <cpptest.h>

class GstTestSuite : public Test::Suite
{
    public:

        GstTestSuite()
        {
            TEST_ADD(GstTestSuite::init_test)
                TEST_ADD(GstTestSuite::start_video)
                TEST_ADD(GstTestSuite::stop_video)
                TEST_ADD(GstTestSuite::start_stop_video)
                TEST_ADD(GstTestSuite::start_mono_audio)
                TEST_ADD(GstTestSuite::stop_mono_audio)
                TEST_ADD(GstTestSuite::start_stop_mono_audio)
                TEST_ADD(GstTestSuite::start_stereo_audio)
                TEST_ADD(GstTestSuite::stop_stereo_audio)
                TEST_ADD(GstTestSuite::start_stop_stereo_audio)
               
                TEST_ADD(GstTestSuite::start_multi_audio)
                TEST_ADD(GstTestSuite::stop_multi_audio)
                TEST_ADD(GstTestSuite::start_stop_multi_audio)
                
              
                //TEST_ADD(GstTestSuite::start_multi_rtp_audio)
                /*
                TEST_ADD(GstTestSuite::stop_multi_rtp_audio)
                TEST_ADD(GstTestSuite::start_stop_multi_rtp_audio)
                */
                
                
        }

        // some tests

    protected:
        virtual void setup();       // setup resources common to all tests  
        virtual void tear_down();   // destroy common resources

    private:
        void block();       // not a unit test
        void init_test();
        void start_video();
        void stop_video();
        void start_stop_video();
        void start_mono_audio();
        void stop_mono_audio();
        void start_stop_mono_audio();
        void start_stereo_audio();
        void stop_stereo_audio();
        void start_stop_stereo_audio();
        void start_multi_audio();
        void stop_multi_audio();
        void start_stop_multi_audio();
        void start_multi_rtp_audio();
        void stop_multi_rtp_audio();
        void start_stop_multi_rtp_audio();
};

#endif // _GST_TEST_SUITE_H_

