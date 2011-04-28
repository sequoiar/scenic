/* gutil.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __GUTIL_H__
#define __GUTIL_H__

class _GstElement;

namespace gutil {
    /// ms to run - 0 is forever
    void runMainLoop(int ms);
    int killMainLoop(void *data = 0);
    void init_gst_gtk(int argc, char **argv);
    void init_gst(int argc, char **argv);
    void initAudioCapsFilter(_GstElement *capsfilter, int numChannels);
    bool has_display();
}

#endif // __GUTIL_H__

