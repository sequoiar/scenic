/* msgThread.h
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

#ifndef __PYTHON_THREAD_H__
#define __PYTHON_THREAD_H__

#include <boost/python.hpp>
#include "msgThread.h"

class dictMessageHandler
{
    public:
        
        virtual boost::python::dict cb(boost::python::dict d);

    virtual ~dictMessageHandler(){}
};

/** template specialization of BaseThread with MapMsg */
class PythonThread
    : public MsgThread
{
    dictMessageHandler msgH_;
    public:
        PythonThread(dictMessageHandler msgH):msgH_(msgH){}
        int main(){

               return 0; 
        }
};

#endif

