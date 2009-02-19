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
}


int gutil::killMainLoop(gpointer /*data*/)
{
    if (loop_)
        g_main_loop_quit(loop_);

    LOG_DEBUG("Quitting...");
    return FALSE;       // won't be called again
}


/// ms to run - 0 is forever
void gutil::runMainLoop(unsigned int ms)
{
    //   std::cout.flush();
    //   std::cout << filename << ":" << function << ":" << lineNumber
    //             << ": in g_main_loop for` " << ms << " milliseconds" << std::endl;
    loop_ = g_main_loop_new (NULL, FALSE);                       
    if(ms)
        g_timeout_add(ms, static_cast<GSourceFunc>(gutil::killMainLoop),
                NULL);
    g_main_loop_run(loop_);
    g_main_loop_unref(loop_);
}

