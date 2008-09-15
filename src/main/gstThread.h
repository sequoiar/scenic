/* GTHREAD-QUEUE-PAIR - Library of GstThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __GST_THREAD__
#define __GST_THREAD__
#include "gutil/baseThread.h"
#include "gutil/strIntFloat.h"

typedef QueuePair_<MapMsg> QueuePair;
class GstThread
    : public BaseThread<MapMsg>
{
    public:
        GstThread(){}
    protected:
        virtual bool audio_start(MapMsg& msg) = 0;
        virtual bool audio_stop(MapMsg& msg) = 0;
        virtual bool video_start(MapMsg& msg) = 0;
        virtual bool video_stop(MapMsg& msg) = 0;

    private:

        int main();

        GstThread(const GstThread&); //No Copy Constructor
        GstThread& operator=(const GstThread&); //No Assignment Operator
};

#endif
