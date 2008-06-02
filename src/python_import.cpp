#include <boost/python.hpp>


#include "hello/hello.h"

#include "gst/videoSender.h"

using namespace boost::python;


#define PROMPT  "gp: >> "


#ifndef __GROUND_LOOP__
int GROUND_LOOP(int result, int argc, char* argv[])
{
    if (result == -1)
        return 0;
    return 1;
}
#endif

BOOST_PYTHON_MODULE(hello)
{
    class_<hello>("hello")
    .def("greet", &hello::greet)
    .def("set_name",&hello::set_name);
}




// Default variable overloads
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(VideoSender_init_overloads, init, 0, 3)


BOOST_PYTHON_MODULE(VideoSender)
{
    class_<VideoSender>("VideoSender")
    .def("init", &VideoSender::init, VideoSender_init_overloads())
    .def("start", &VideoSender::start)
    .def("stop",  &VideoSender::stop);


}






void BOOST_PY_IMPORT()
{
    PyImport_AppendInittab((char*)"VideoSender",&initVideoSender);
    PyImport_AppendInittab((char*)"hello",&inithello);
}

void PYTHON_EXEC_IMPORT(object mm, object mn)
{
    if(mm)
    {
        exec("import hello; from hello import *",mm,mn);
        exec("import VideoSender; from VideoSender import *",mm,mn);
    }
}

