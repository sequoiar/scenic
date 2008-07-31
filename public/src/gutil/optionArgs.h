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
 *      And more.#ifndef __BASE_THREAD_H__
 #define __BASE_THREAD_H__


 */
#ifndef __OPTION_ARGS_H__
#define __OPTION_ARGS_H__


#include "config.h"
#include <vector>
#include <glib.h>
#include "baseModule.h"

class OptionArgs
{
    typedef std::vector<GOptionEntry> Options;
//	std::vector<char **> str_dump;
    Options options;
    public:
        void add(BaseModule::ArgList);
        void add(BaseArg*);

//	void add(bool *,const char*,char, const char*);
//	void add(int *,const char*,char, const char*,const char*);
//	void add(char **,const char*,char,const char*,const char*);

        GOptionEntry* getArray();

        GOptionEntry* pA;
        int parse(int argc, char **argv);

        OptionArgs()
            : options(), pA(0){}
        ~OptionArgs();
    private:
        OptionArgs(const OptionArgs&); //No Copy Constructor
        OptionArgs& operator=(const OptionArgs&); //No Assignment Operator
};


#endif
