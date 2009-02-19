/* pyCallback.h
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

#ifndef __PY_CALLBACK_H__
#define __PY_CALLBACK_H__

#include <boost/python.hpp>
#include "msgThread.h"

///python inheritable callback class
struct dictMessageHandler 
{
    virtual boost::python::dict cb(boost::python::dict d)=0; 
    virtual ~dictMessageHandler(){}
};

///wrapper for python callback handler
struct HandlerWrapper 
    : dictMessageHandler 
{
    HandlerWrapper(PyObject *p) : self(p) {}

    /// Call the virtual function in python                         
    boost::python::dict cb(boost::python::dict d)
    {
        return boost::python::call_method<boost::python::dict>(self,"cb",d);
    }
    PyObject *self;

    ///No Copy Constructor
    HandlerWrapper(const HandlerWrapper& );
    ///No Assignment Operator
    HandlerWrapper& operator=(const HandlerWrapper&); 

};


//http://wiki.python.org/moin/boost.python/ExportingClasses
#endif

