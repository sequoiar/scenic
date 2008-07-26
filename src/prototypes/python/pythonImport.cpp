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
#include <Python.h>
#include <boost/python.hpp>

#include "hello/hello.h"

using namespace boost::python;


BOOST_PYTHON_MODULE(libpyhello)
{
    class_ < Hello > ("Hello").def("greet", &Hello::greet).def("set_name"
                                                               ,&Hello::set_name);
}


