#include <boost/python.hpp>


#include "hello/hello.h"

#include "gst/videoSender.h"

using namespace boost::python;


BOOST_PYTHON_MODULE(hello)
{
    class_<hello>("hello")
    .def("greet", &hello::greet)
    .def("set_name",&hello::set_name);
}

BOOST_PYTHON_MODULE(VideoSender)
{
    class_<VideoSender>("VideoSender")
    .def("init", &VideoSender::init)
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

