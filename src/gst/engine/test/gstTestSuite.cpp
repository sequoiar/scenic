#include <cstdlib>
#include <cstring>
#include <glib.h>
#include "gstTestSuite.h"
#include "gutil.h"

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


int GstTestSuite::killMainLoop(gpointer data)
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
    gutil::runMainLoop(testLength_);
}

bool GstTestSuite::areValidArgs(int argc, char **argv)
{
    bool isValid = (argc == 2);
    if (!isValid)
        return isValid;
    int pid = atoi(argv[1]);
    isValid = (argc == 2) && (strlen(argv[1]) == 1) && ((pid == 0) || (pid == 1));
    return isValid;
}

