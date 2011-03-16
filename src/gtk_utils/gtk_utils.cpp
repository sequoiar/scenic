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

#include <gst/gst.h>
#include <gtk/gtk.h>
#include "util/logWriter.h"
#include "util/sigint.h"
#include "gtk_utils.h"
    
// extend namespace gutil
namespace gutil {
    int checkSignal(gpointer data = NULL);
}

int gutil::killMainLoop(gpointer /*data*/)
{
    static bool called = false;
    if (not called)
    {
        gtk_main_quit();

        LOG_DEBUG("Quitting...");
        called = true;
    }
    return FALSE;       // won't be called again
}

int gutil::checkSignal(gpointer /*data*/)
{
    if (signal_handlers::signalFlag())
    {
        killMainLoop();
        return FALSE; // won't be called again
    }

    return TRUE; // keep calling
}

/// ms to run - 0 is forever
void gutil::runMainLoop(int ms)
{
    /// the gtk main loop will die after ms has elapsed
    if (ms != 0)
        g_timeout_add(ms, static_cast<GSourceFunc>(gutil::killMainLoop), NULL);

    // FIXME: this isn't very smart
    // poll signal status every quarter second
    g_timeout_add(300 /*ms*/,
            static_cast<GSourceFunc>(gutil::checkSignal),
            NULL);

    gtk_main();
}

void gutil::init_gst_gtk(int argc, char **argv)
{
    // must initialise the threading system before using any other GLib funtion
    if (!g_thread_supported ())
        g_thread_init (NULL);

    gst_init(&argc, &argv);
    if (getenv("DISPLAY") != NULL)
        gtk_init(&argc, &argv);
    else
        LOG_DEBUG("DISPLAY variable has not been set");
}

