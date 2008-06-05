// hello.h
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

/** \brief hello simple example class for pyInterpreter.
 *          Instantiate, set_name then call greet.
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */

#ifndef _HELLO_H_
#define _HELLO_H_

#include <string>

class hello
{
    std::string s;
public:
    const char * greet();
    void set_name(char const* n);
};

#endif //_HELLO_H_
