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

#include <glib.h>
#include <iostream>
#include "gutil/baseThread.h"
#include "gutil/strIntFloat.h"
#include "gstThread.h"

class VideoSender;
class AudioSender;

typedef QueuePair_<MapMsg> QueuePair;
class GstSenderThread
    : public GstThread
{
    public:
        GstSenderThread()
            : vsender_(0), asender_(0){}
        ~GstSenderThread();
    private:
        VideoSender* vsender_;
        AudioSender* asender_;

        bool audio_start(MapMsg& msg);
        bool audio_stop(MapMsg& msg);
        bool video_start(MapMsg& msg);
        bool video_stop(MapMsg& msg);

        GstSenderThread(const GstSenderThread&); //No Copy Constructor
        GstSenderThread& operator=(const GstSenderThread&); //No Assignment Operator
};


