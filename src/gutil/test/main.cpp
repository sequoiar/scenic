// headerGPL.c
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
 *      Just the License GPL 3+
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */
#include "config.h"
#include "optionArgs.h"
#include "logWriter.h"

int main (int argc, char **argv)
{
    int result = 0;
    char* str = 0;
    bool b = false;
    OptionArgs options;

    options.add(new BoolArg(&b, "flag", 'f', "Set f")) ;
    options.add(new IntArg(&result, "try", 't', "try it out", "pass an int")) ;
    options.add(new StrArg( &str, "str", 's', "try it out", "pass a string")) ;
    options.parse(argc, argv);

    LOG(b);
    LOG(result);
    LOG(str);
    return 0;
}


