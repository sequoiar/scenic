/* threadWrap.h
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
#ifndef _THREADWRAP_H_
#define _THREADWRAP_H_

#include <Python.h>
#include <boost/python.hpp>
#include "msgThreadFactory.h"
#include "pythonThread.h"


class MsgWrapConfig
{
    public:
        virtual MsgThread* GetMsgThread(){ return NULL;} ;
        virtual ~MsgWrapConfig(){}
};


class TcpWrapConfig : public MsgWrapConfig
{
    int port_;
    int log_;

    public:
        TcpWrapConfig(int port,bool log):port_(port),log_(log){}
        MsgThread* GetMsgThread(){ return MsgThreadFactory::Tcp(port_,log_);}
};


class GstWrapConfig : public MsgWrapConfig
{
    public:
        GstWrapConfig(){}
        MsgThread* GetMsgThread(){ return MsgThreadFactory::Gst(false);}
};

class ThreadWrap
{
    std::auto_ptr<MsgThread> thread_;
    QueuePair &q_;
    PythonThread pyThread_;

    public:

        ThreadWrap(MsgWrapConfig* conf,dictMessageHandler* hd): 
            thread_(conf->GetMsgThread()),q_(thread_->getQueue()),pyThread_(q_,hd)
        {   
            PyEval_InitThreads();
            pyThread_.run();
            thread_->run();
        }
       
        bool send(boost::python::dict dt)
        {     
            MapMsg m;
            boost::python::list l = dt.items();
            int n = boost::python::extract<int>(l.attr("__len__")());
            LOG_DEBUG(n);
                
            for ( int i = 0; i < n; i++ )
            {
                std::string skey,sval;
                tuple val = (boost::python::extract<boost::python::tuple>(l[i]));
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
     
        //TODO remove - now implemented pythonThread
        boost::python::dict
        getMsg(int ){ return boost::python::dict();}
        
};



#endif
