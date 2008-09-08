#include <stdlib.h>
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
    else {
        std::cerr << "Id must be 0 or 1." << std::endl;
        exit(1);
    }
}


gboolean GstTestSuite::killMainLoop(gpointer data)
{
    GMainLoop *loop = static_cast<GMainLoop *>(data);
    g_main_loop_quit(loop);
    return FALSE;       // won't be called again
}


void GstTestSuite::block(const char * filename, const char *function, long lineNumber)
{
    std::cout.flush();
    std::cout << filename << ":" << function << ":" << lineNumber
              << ": blocking for " << testLength_ << " milliseconds" << std::endl;
    GMainLoop *loop;                                             \
    loop = g_main_loop_new (NULL, FALSE);                       \
    g_timeout_add(testLength_, static_cast<GSourceFunc>(GstTestSuite::killMainLoop),
                  static_cast<gpointer>(loop));
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    //std::cin.get()
}


