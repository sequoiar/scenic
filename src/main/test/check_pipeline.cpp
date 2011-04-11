#include "gutil/gutil.h"
#include "gst/pipeline.h"

int main(int argc, char **argv)
{
    gutil::init_gst(argc, argv);
    Pipeline pipeline;
    pipeline.start();
    gutil::runMainLoop(1000);

    return 0;
}
