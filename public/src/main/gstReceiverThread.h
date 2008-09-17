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
#include "baseThread.h"
#include "gstThread.h"

class VideoReceiver;
class AudioReceiver;

class GstReceiverThread
    : public GstThread
{
    public:
        GstReceiverThread()
            : vreceiver_(0), areceiver_(0){}
        ~GstReceiverThread();
    private:
        VideoReceiver* vreceiver_;
        AudioReceiver* areceiver_;

        bool audio_start(MapMsg& msg);
        bool audio_stop(MapMsg& msg);
        bool video_start(MapMsg& msg);
        bool video_stop(MapMsg& msg);

        GstReceiverThread(const GstReceiverThread&); //No Copy Constructor
        GstReceiverThread& operator=(const GstReceiverThread&); //No Assignment Operator
};


