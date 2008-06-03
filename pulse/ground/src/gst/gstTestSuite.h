
// gstTestSuite.h
//
// Declares and registers a series of unit tests. Note that currently, these tests are quite simple. 
// A more comprehensive/detailed testsuite would have to cover specific
// usecases and look more like client code. The current set of tests just does a quick probe of our 
// media interface.

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
                
                TEST_ADD(GstTestSuite::start_dv)
                TEST_ADD(GstTestSuite::stop_dv)
                TEST_ADD(GstTestSuite::start_stop_dv)
                
                TEST_ADD(GstTestSuite::start_v4l)
                TEST_ADD(GstTestSuite::stop_v4l)
                TEST_ADD(GstTestSuite::start_stop_v4l)
                
                TEST_ADD(GstTestSuite::start_v4l_rtp)
                TEST_ADD(GstTestSuite::stop_v4l_rtp)
                TEST_ADD(GstTestSuite::start_stop_v4l_rtp)

                TEST_ADD(GstTestSuite::start_dv_rtp)
                TEST_ADD(GstTestSuite::stop_dv_rtp)
                    TEST_ADD(GstTestSuite::start_stop_dv_rtp)

                    TEST_ADD(GstTestSuite::start_1ch_audio)
                    TEST_ADD(GstTestSuite::stop_1ch_audio)
                    TEST_ADD(GstTestSuite::start_stop_1ch_audio)

                    TEST_ADD(GstTestSuite::start_2ch_audio)
                    TEST_ADD(GstTestSuite::stop_2ch_audio)
                    TEST_ADD(GstTestSuite::start_stop_2ch_audio)

                    TEST_ADD(GstTestSuite::start_6ch_audio);
                    TEST_ADD(GstTestSuite::stop_6ch_audio)
                    TEST_ADD(GstTestSuite::start_stop_6ch_audio)

                    TEST_ADD(GstTestSuite::start_8ch_audio)
                    TEST_ADD(GstTestSuite::stop_8ch_audio)
                    TEST_ADD(GstTestSuite::start_stop_8ch_audio)
                  
                    TEST_ADD(GstTestSuite::start_8ch_comp_rtp_audio)
                    TEST_ADD(GstTestSuite::stop_8ch_comp_rtp_audio)
                    TEST_ADD(GstTestSuite::start_stop_8ch_comp_rtp_audio)
#if 0
                    TEST_ADD(GstTestSuite::start_8ch_uncomp_rtp_audio)
                    TEST_ADD(GstTestSuite::stop_8ch_uncomp_rtp_audio)
                    TEST_ADD(GstTestSuite::start_stop_8ch_uncomp_rtp_audio)
#endif
            }

            void set_id(int id);

            // some tests

        protected:
            virtual void setup();       // setup resources common to all tests  
            virtual void tear_down();   // destroy common resources

        private:
            static int testCounter_;
        int id_;
        void block(); // inline
        void init_test();

        void start_video();
        void stop_video();
        void start_stop_video();
        
        void start_v4l();
        void stop_v4l();
        void start_stop_v4l();
        
        void start_v4l_rtp();
        void stop_v4l_rtp();
        void start_stop_v4l_rtp();

        void start_dv();
        void stop_dv();
        void start_stop_dv();

        void start_dv_rtp();
        void stop_dv_rtp();
        void start_stop_dv_rtp();

        void start_1ch_audio();
        void stop_1ch_audio();
        void start_stop_1ch_audio();

        void start_2ch_audio();
        void stop_2ch_audio();
        void start_stop_2ch_audio();

        void start_6ch_audio();
        void stop_6ch_audio();
        void start_stop_6ch_audio();

        void start_8ch_audio();
        void stop_8ch_audio();
        void start_stop_8ch_audio();

        void start_8ch_comp_rtp_audio();
        void stop_8ch_comp_rtp_audio();
        void start_stop_8ch_comp_rtp_audio();
        
        void start_8ch_uncomp_rtp_audio();
        void stop_8ch_uncomp_rtp_audio();
        void start_stop_8ch_uncomp_rtp_audio();
};

inline
void GstTestSuite::block()
{
    char c;
    std::cout.flush();
    std::cout << __FILE__ << ":" << __LINE__ << ": blocking, enter any key." << std::endl;
    std::cin >> c;
}

#endif // _GST_TEST_SUITE_H_

