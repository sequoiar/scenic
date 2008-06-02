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
        void block(); // inline
        void init_test();

        void start_video();
        void stop_video();
        void start_stop_video();

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

        void start_8ch_rtp_audio();
        void stop_8ch_rtp_audio();
        void start_stop_8ch_rtp_audio();
};

inline
void GstTestSuite::block()
{
    char c;
    std::cout.flush();
    std::cout << __FILE__ << ": blocking, enter any key." << std::endl;
    std::cin >> c;
}

#endif // _GST_TEST_SUITE_H_

