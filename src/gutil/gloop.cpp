/* gutil.h
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

#include "util.h"

#include "gutil.h"
    
// extend namespace gutil
namespace gutil {
    static GMainLoop *loop_ = 0;
    int checkSignal(gpointer data = NULL);
}


int gutil::killMainLoop(gpointer /*data*/)
{
    if (loop_)
        g_main_loop_quit(loop_);

    LOG_DEBUG("Quitting...");
    return FALSE;       // won't be called again
}

int gutil::checkSignal(gpointer /*data*/)
{
    if (signalFlag())
    {
        killMainLoop(NULL);
        //THROW_END_THREAD("Got signal flag in gloop");
        return FALSE; // won't be called again
    }

    return TRUE; // keep calling
}

/// ms to run - 0 is forever
void gutil::runMainLoop(int ms)
{
    loop_ = g_main_loop_new (NULL, FALSE);                       
    if(ms)
        g_timeout_add(ms, static_cast<GSourceFunc>(gutil::killMainLoop),
                NULL);

    g_timeout_add(1000 /*ms*/,  // poll signal status every second
            static_cast<GSourceFunc>(gutil::checkSignal),
            NULL);

    g_main_loop_run(loop_);
    g_main_loop_unref(loop_);
    loop_ = 0;
}

