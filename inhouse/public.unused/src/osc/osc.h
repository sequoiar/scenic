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

#include <lo/lo.h>
#include <string>
#include <vector>
#include <iostream>
#include "baseThread.h"
#include "oscMessage.h"


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

        static void liblo_error(int, const char *, const char *){}
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

