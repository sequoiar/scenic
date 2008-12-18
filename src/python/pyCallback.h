/* pyCallback.h
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

#ifndef __PY_CALLBACK_H__
#define __PY_CALLBACK_H__

#include <boost/python.hpp>
#include "msgThread.h"


struct dictMessageHandler 
{
    virtual boost::python::dict cb(boost::python::dict d)=0; 
};

struct HandlerWrapper 
    : dictMessageHandler 
{
    HandlerWrapper(PyObject *p) : self(p) {}
    boost::python::dict cb(boost::python::dict d)
    {
        // Call the virtual function in python                         
        return boost::python::call_method<boost::python::dict>(self,"cb",d);
    }
    PyObject *self;
};


//http://wiki.python.org/moin/boost.python/ExportingClasses
#endif

