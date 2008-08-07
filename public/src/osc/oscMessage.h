
/* headerGPL.c
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
 */

/** \file
 *      Osc Message and Thread
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */

#ifndef __OSC_MESSAGE_H__
#define __OSC_MESSAGE_H__

#include "gutil/baseThread.h"
#include <lo/lo.h>
#include <string>
#include <vector>
#include <iostream>

class OscMessage
{
    public:
        OscMessage(const char *p, const char *t, lo_arg ** v, int c, void *d);
        OscMessage()
            : path_(), types_(), args_(), argc_(0), data_(0){}

        OscMessage(const OscMessage& in);
        OscMessage& operator=(const OscMessage& in);

        std::string path() { return path_; }
        bool pathIsSet() { return !path_.empty(); }
        bool pathEquals(std::string str) { return path_ == str; }

        lo_message *init_msg(lo_message *msg);
        void print();

        bool argEquals(std::string str, int idx) { return args_[idx].equals(str); }
        bool argEquals(int val, int idx) { return args_[idx].equals(val); }

    private:
        class LoArg
        {
            public:
                LoArg(std::string pchar, int index, lo_arg * a);
                void print() const;
                bool equals(std::string str);
                bool equals(int val);
                lo_type type_;
                int i_;
                std::string s_;
        };

        typedef std::vector < LoArg > OscArgs;
        std::string path_, types_;
        OscArgs args_;
        int argc_;
        void *data_;
};
#endif

