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

/** \file
 *      This file gets included in python module
 *
 *
 *      Exposes object modules to python interpreter.
 *
 */

#include <iostream>
#include <boost/python.hpp>

#include "hello/hello.h"
#include "osc/osc.h"

using namespace boost::python;

#define PROMPT  "gp: >> "

#ifndef __GROUND_LOOP__
int ground_loop(int result)
{
	if (result == -1)
		return 0;
	return 1;
}
#endif

#ifndef __GROUND_INIT__
int ground_init(int argc, char *argv[])
{

	return 0;
}
#endif

BOOST_PYTHON_MODULE(Hello)
{
	class_ < Hello > ("Hello").def("greet", &Hello::greet).def("set_name", &Hello::set_name);
}

BOOST_PYTHON_MODULE(OscThread)
{
	class_ < OscThread > ("OscThread").def("getQueue", &OscThread::getQueue).def("run", &OscThread::run);

}

BOOST_PYTHON_MODULE(OscMessage)
{
	class_ < OscMessage > ("OscMessage").def_readonly("path", &OscMessage::path);

}

BOOST_PYTHON_MODULE(QueuePairOfOscMessage)
{
	class_ < QueuePairOfOscMessage > ("QueuePairOfOscMessage").def("copy_timed_pop",
	                                                               &QueuePairOfOscMessage::
	                                                                copy_timed_pop).def("push",
	                                                                                    &QueuePairOfOscMessage::
	                                                                                     push);
}

void BOOST_PY_IMPORT()
{
	PyImport_AppendInittab((char *) "Hello", &initHello);
	PyImport_AppendInittab((char *) "OscThread", &initOscThread);
	PyImport_AppendInittab((char *) "QueuePairOfOscMessage", &initQueuePairOfOscMessage);
	PyImport_AppendInittab((char *) "OscMessage", &initOscMessage);
}

void PYTHON_EXEC_IMPORT(object mm, object mn)
{
	if (mm) {
		exec("import Hello; from Hello import *", mm, mn);
		exec("import OscThread; from OscThread import *", mm, mn);
		exec("import QueuePairOfOscMessage; from QueuePairOfOscMessage import *", mm, mn);
		exec("import OscMessage; from OscMessage import *", mm, mn);
	}
}

// Default variable overloads
//BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(VideoLocal_init_overloads, init, 0, 3)

/*
   BOOST_PYTHON_MODULE(VideoLocal)
   {
   	class_<VideoLocal>("VideoLocal")
   	.def("init", &VideoLocal::init)
   	.def("start", &VideoLocal::start)
   	.def("stop",  &VideoLocal::stop);

   }
 */
