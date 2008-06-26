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
#include "thread/baseThread.h"
#include "thread/message.h"

using namespace boost::python;


#define PROMPT  "gp: >> "



class Thread : public BaseThread<Message>
{
    int main();
};

int Thread::main()
{
	static Message r(message::ok);
    int count=0;
    while(1) 
    { 
        Message& f = *queue_pair_pop<Message*>(queue);
        std::cout << message::str[f.type];

		queue_pair_push(queue,&r);
		if(count++ == 1000) 
		{
			static Message f(message::quit);			
			queue_pair_push(queue,&f);
    	    break;
		}
    }
return 0; 
}

#ifndef __GROUND_LOOP__
int ground_loop(int result)
{
    if (result == -1)
        return 0;
    return 1;
}
#endif

#ifndef __GROUND_INIT__
int ground_init(int argc, char* argv[]) 
{ 
    
    return 0;
}
#endif

BOOST_PYTHON_MODULE(Hello)
{
    class_<Hello>("Hello")
    .def("greet", &Hello::greet)
    .def("set_name",&Hello::set_name);
}

BOOST_PYTHON_MODULE(Thread)
{
    class_<Thread>("Thread")
    .def("getInvertQueue",&Thread::getInvertQueue)
    .def("run",&Thread::run);
     
}

BOOST_PYTHON_MODULE(Message)
{
    class_<Message>("Message",init<int>())
        .def("getInt",&Message::getInt);

}

typedef QueuePair_<Message> QueuePair;

BOOST_PYTHON_MODULE(QueuePair)
{
    class_<QueuePair>("QueuePair")
        .def("copy_timed_pop",&QueuePair::copy_timed_pop)
        .def("push",&QueuePair::push);
}






void BOOST_PY_IMPORT()
{
    PyImport_AppendInittab((char*)"Hello",&initHello);
    PyImport_AppendInittab((char*)"Thread",&initThread);
    PyImport_AppendInittab((char*)"QueuePair",&initQueuePair);
    PyImport_AppendInittab((char*)"Message",&initMessage);
}

void PYTHON_EXEC_IMPORT(object mm, object mn)
{
    if(mm)
    {
        exec("import Hello; from Hello import *",mm,mn);
        exec("import Thread; from Thread import *",mm,mn);
        exec("import QueuePair; from QueuePair import *",mm,mn);
        exec("import Message; from Message import *",mm,mn);
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

