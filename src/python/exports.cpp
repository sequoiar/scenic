/* exports.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/** \file
 *      This file exposes class to python 
 *
 *
 *      Exposes object modules to python interpreter.
 *
 */

#include <iostream>
#include <Python.h>
#include <boost/python.hpp>


using namespace boost::python;

class Hello
{
public:
    Hello(){}
    const char* greet(){ return "hello";}


};



#if 0
BOOST_PYTHON_MODULE(libpyboostskel)
{
    class_ < Hello > ("Hello")
        .def("greet", &Hello::greet)
    ;
}
#endif
#include "tcp/tcpThread.h"
#include "threadWrap.h"
#if 0
#endif
BOOST_PYTHON_MODULE(libmsgthreads)
{
    
    class_ < MsgWrapConfig > ("MsgWrapConfig", no_init)
        ;
    class_ < TcpWrapConfig, bases<MsgWrapConfig> >("TcpWrapConfig",init <int, bool> ())
        ;
    class_ < ThreadWrap > ("ThreadWrap",init < MsgWrapConfig* > ())
        .def("getMsg", &ThreadWrap::getMsg)
        .def("send", &ThreadWrap::send)
        ;
}



        //.def("getQueue", &TcpThread::getQueue)















#if 0
class Handler 
{   
public:
    virtual std::string disp(MapMsg& /*msg*/){return "";}
    Handler(MapMsg& msg) { disp(msg);}
    virtual ~Handler(){}

};

class ASubscriber : public msg::Subscriber
{
    Handler &h_;
    public:
        ASubscriber(Handler& h)
            : h_(h)
        {}
void operator()(MapMsg& msg){ h(msg); }
};
#endif




