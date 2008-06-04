
// python_import.cpp
// Copyright 2008 Koya Charles & Tristan Matthews 
//     
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//


#include <boost/python.hpp>


#include "hello/hello.h"

#include "gst/videoSender.h"

using namespace boost::python;


#define PROMPT  "gp: >> "


#ifndef __GROUND_LOOP__
int _ground_loop(int result)
{
    if (result == -1)
        return 0;
    return 1;
}
#endif

#ifndef __GROUND_INIT__
int _ground_init(int argc, char* argv[]) { return 0;}
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

