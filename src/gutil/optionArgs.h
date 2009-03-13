/* optionArgs.c
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

#ifndef __OPTION_ARGS_H__
#define __OPTION_ARGS_H__

#include <glib.h>
#include <string>
#include <list>

#include "mapMsg.h"

/// command line handler parses main(argc,argv) and provides --help
class OptionArgs
{
    public:
        void addBool(const char *l_arg, char, const char *desc);
        void addInt(const char *l_arg, char, const char *desc, const char *arg_desc);
        void addString(const char *l_arg, char, const char *desc, const char *arg_desc);
        void parse(int argc, char **argv);

        OptionArgs()
            : store(), options_(), pA_(0){}
        ~OptionArgs();

        ///expose map interface
        StrIntFloat& operator[] (const std::string &key) { return store[key]; }

    private:
        ///client assess store filled after parse is called
        MapMsg store;
        ///The glib level options interface
        typedef std::pair<GOptionEntry,char**> OptPair;
        typedef std::list<OptPair> Options;
        GOptionEntry* getArray();

        /// No Copy Constructor
        OptionArgs(const OptionArgs&); 
        /// No Assignment Operator
        OptionArgs& operator=(const OptionArgs&); 

        Options options_;
        GOptionEntry* pA_;
};

#endif

