/* optionArgs.c
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

#ifndef __OPTION_ARGS_H__
#define __OPTION_ARGS_H__

#include <glib.h>
#include <string>
#include <vector>

/// Base class of Arguments used in command line parsing
class BaseArg
{
    public:
        BaseArg(char t, std::string l, char s, std::string d, std::string a)
            : type(t), l_arg(l), desc(d), arg_desc(a), s_arg(s){}

        virtual ~BaseArg() {}

    protected:
        friend class OptionArgs;
        char type;
        std::string l_arg, desc, arg_desc;
        char s_arg;
};

/// Integer argument
class IntArg
    : public BaseArg
{
    public:
        int* arg;
        IntArg(int* i, std::string l, char s, std::string d, std::string a)
            : BaseArg('i', l, s, d, a), arg(i){}

    private:
        /// No Copy Constructor
        IntArg(const IntArg&); //No Copy Constructor
        /// No Assignment Operator
        IntArg& operator=(const IntArg&); //No Assignment Operator
};


/// Boolean argument
class BoolArg
    : public BaseArg
{
    public:
        bool* arg;
        BoolArg(bool* b, std::string l, char s, std::string d)
            : BaseArg('b', l, s, d, std::string()), arg(b){}

    private:
        /// No Copy Constructor
        BoolArg(const BoolArg&); //No Copy Constructor
        /// No Assignment Operator
        BoolArg& operator=(const BoolArg&); //No Assignment Operator
};


/// String argument
class StringArg
    : public BaseArg
{
    public:
        char** arg ;
        StringArg(char** ppc, std::string l, char s, std::string d, std::string a)
            : BaseArg('s', l, s, d, a), arg(ppc){}

    private:
        /// No Copy Constructor
        StringArg(const StringArg&); //No Copy Constructor
        /// No Assignment Operator
        StringArg& operator=(const StringArg&); //No Assignment Operator
};


/// command line handler
class OptionArgs
{
    public:
        /// Add a BaseArg derived option 
        void add(BaseArg*);
        /// input command line arguments
        int parse(int argc, char **argv);

        OptionArgs()
            : options_(), pA_(0){ options_.clear();}
        ~OptionArgs();

    private:
        typedef std::vector<GOptionEntry> Options;
        GOptionEntry* getArray();

        /// No Copy Constructor
        OptionArgs(const OptionArgs&); 
        /// No Assignment Operator
        OptionArgs& operator=(const OptionArgs&); 

        Options options_;
        GOptionEntry* pA_;
};

#endif

