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

#ifndef __OSC_THREAD_H__
#define __OSC_THREAD_H__

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

        const char* path() { return path_.c_str(); }
        bool pathIsSet() { return !path_.empty(); }
        bool pathEquals(const char *str) { return path_ == str; }

        lo_message *init_msg(lo_message *msg);
        void print();

        bool argEquals(const char *str, int idx) { return args_[idx].equals(str); }

    private:
        class LoArg
        {
            public:
                LoArg(const char *pchar, int index, lo_arg * a)
                    : type_(static_cast<lo_type>(pchar[index])), i_(0), s_()
                {
                    switch ((char) type_)
                    {
                        case 's':
                            {
                                s_ = static_cast < char *>(&(a->s));
                                break;
                            }
                        case 'i':
                            {
                                i_ = static_cast < int >(a->i);
                                break;
                            }
                    }
                }

                void print() const;

                bool equals(const char *str)
                {
                    if ((char) type_ == 's')
                        return s_ == str;

                    return false;
                }
                
                bool equals(int val)
                {
                    if ((char) type_ == 'i')
                        return i_ == val;

                    return false;
                }


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


typedef QueuePair_ < OscMessage > OscQueue;

class OscThread
: public BaseThread < OscMessage >
{
    public:
        OscThread();
        void set_local_port(std::string p){ local_port_ = p; }
    private:
        int main();
        bool ready();

        static int generic_handler_static(const char *path, const char *types, lo_arg ** argv,
                int argc, void *data,
                void *user_data);

        int generic_handler(const char *path, const char *types, lo_arg ** argv, int argc,
                void *data);

        static void liblo_error(int num, const char *msg, const char *path){}
        bool send(OscMessage & osc);

    private:
        std::string local_port_;
        std::string remote_port_;
        std::string remote_host_;
        const char * arg_local_port_;
        bool running_;
        OscThread(const OscThread&); //No Copy Constructor
        OscThread& operator=(const OscThread&); //No Assignment Operator
};

#endif

