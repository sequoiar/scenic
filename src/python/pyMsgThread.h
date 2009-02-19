/* threadWrap.h
 * Copyright (C) 2009 Société des arts technologiques (SAT)
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

#ifndef _THREADWRAP_H_
#define _THREADWRAP_H_

#include <Python.h>
#include <boost/python.hpp>
#include "msgThreadFactory.h"
#include "pyCallback.h"

/// Used my ThreadWrap to poll and post to python
class PythonThread
    : public MsgThread
{
    QueuePair& q_;
    dictMessageHandler& msgH_;

    public:
        PythonThread(QueuePair& q,dictMessageHandler* msgH):q_(q),msgH_(*msgH){}
        int main();
        
};

/// non msgThread to be derived
class MsgWrapConfig
{
    public:
        virtual MsgThread* GetMsgThread(){ return NULL;} ;
        virtual ~MsgWrapConfig(){}
};

/// TCP
class TcpWrapConfig : public MsgWrapConfig
{
    int port_;
    int log_;

    public:
        TcpWrapConfig(int port,bool log):port_(port),log_(log){}
        MsgThread* GetMsgThread(){ return MsgThreadFactory::Tcp(port_,log_);}
};

/// GST
class GstWrapConfig : public MsgWrapConfig
{
    public:
        GstWrapConfig(){}
        MsgThread* GetMsgThread(){ return MsgThreadFactory::Gst(false);}
};

/// Wraps python thread and msgThread derived class
/// @param conf is factory for MsgThreads
/// @param hd should be a python callback
class ThreadWrap
{
    std::auto_ptr<MsgThread> thread_;
    QueuePair &q_;
    PythonThread pyThread_;

    public:
        ThreadWrap(MsgWrapConfig* conf,dictMessageHandler* hd);
       
        bool send(boost::python::dict dt);

};


#endif

