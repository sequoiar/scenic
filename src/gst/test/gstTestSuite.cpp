
#include "gstTestSuite.h"

const int GstTestSuite::NUM_CHANNELS = 8;
const int GstTestSuite::V_PORT = 10010;
const int GstTestSuite::A_PORT = 11010;

void GstTestSuite::setup()
{
    std::cout.flush();
    std::cout << std::endl;
}

void GstTestSuite::tear_down()
{
    // empty
}

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

