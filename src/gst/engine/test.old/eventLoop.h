
// eventLoop.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#define BLOCKING 1

#if BLOCKING
#define BLOCK() eventloop::block(__FILE__, __FUNCTION__, __LINE__)
#else
#define BLOCK()
#endif

#include <glib.h>
#include <iostream>

namespace eventloop 
{
        void block(const char *filename, const char *function, long lineNumber);
        int killMainLoop(void *data);
        const unsigned long long RUNNING_TIME = 60000;
}

int eventloop::killMainLoop(gpointer data)
{
    GMainLoop *loop = static_cast<GMainLoop *>(data);
    g_main_loop_quit(loop);
    return FALSE;       // won't be called again
}


void eventloop::block(const char * filename, const char *function, long lineNumber)
{
    std::cout.flush();
    std::cout << filename << ":" << function << ":" << lineNumber;
    GMainLoop *loop;                                             
    loop = g_main_loop_new (NULL, FALSE);                       

    // UNComment if you want this event loop to end after RUNNING_TIME ms
#if 0
    g_timeout_add(RUNNING_TIME, 
            static_cast<GSourceFunc>(eventloop::killMainLoop), 
            static_cast<void*>(loop));
#endif
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
}

#endif

