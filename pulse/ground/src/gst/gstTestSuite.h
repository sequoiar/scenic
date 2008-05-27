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
        }
        
        // some tests
    
    protected:
        virtual void setup();       // setup resources common to all tests  
        virtual void tear_down();   // destroy common resources

    private:
        void init_test();
        void start_video();
        void stop_video();
};

#endif // _GST_TEST_SUITE_H_
