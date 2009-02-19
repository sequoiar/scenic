/* pyMsgThread.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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

#include "pyMsgThread.h"


static boost::python::dict makeDict(MapMsg& m)
{ 
    boost::python::dict d;
    const std::pair<const std::string, StrIntFloat>* it;
    for(it = m.begin(); it != 0; it = m.next())
    {
        LOG_DEBUG("");
        switch(it->second.get_type())
        {
            case 's':
                d.setdefault(it->first.c_str(),std::string(it->second));
                break;
            case 'i':
                d.setdefault(it->first.c_str(),int(it->second));
                break;
            case 'f':
                d.setdefault(it->first.c_str(),double(it->second));
                break;
            case 'F':
                break;
            case 'e':
                break;
//                default:
//                    THROW_ERROR("Command " << it->first
//                                         << " has unknown type " << it->second.get_type());
        }
    }
    return d;
}


ThreadWrap::ThreadWrap(MsgWrapConfig* conf,dictMessageHandler* hd): 
    thread_(conf->GetMsgThread()),q_(thread_->getQueue()),pyThread_(q_,hd)
{   
    PyEval_InitThreads();
    pyThread_.run();
    thread_->run();
}


bool ThreadWrap::send(boost::python::dict dt)
{     
    MapMsg m;
    boost::python::list l = dt.items();
    int n = boost::python::extract<int>(l.attr("__len__")());
    LOG_DEBUG(n);
        
    for ( int i = 0; i < n; i++ )
    {
        std::string skey,sval;
        boost::python::tuple val = (boost::python::extract<boost::python::tuple>(l[i]));
        skey = boost::python::extract<std::string>(val[0]);
        if(boost::python::extract<std::string>(val[1]).check()){
            m[skey] = boost::python::extract<std::string>(val[1]); 
        }else
        if(boost::python::extract<int>(val[1]).check()){
            m[skey] = boost::python::extract<int>(val[1]); 
        }
        q_.push(m);
        //sval = boost::python::extract<std::string>(val[1]);
        //m[skey] = sval;
        LOG_DEBUG(skey << ">>" << sval);
    }
    return true; 
}


int PythonThread::main()
{
    LOG_INFO("Python Thread");
    for(;;)
    {
        MapMsg m = q_.timed_pop(10);
        if(m["command"].empty())
            continue;
        if(std::string(m["command"]) == "quit")
            break;

        PyGILState_STATE state = PyGILState_Ensure();
        LOG_INFO("makeDictionary");
        msgH_.cb(makeDict(m)); 
        PyGILState_Release(state);

    }
    return 0; 
}


