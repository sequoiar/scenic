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
#include "gutil/stdMsg.h"

#include "gst/videoSender.h"
#include "gst/videoReceiver.h"
#include "gst/videoConfig.h"

typedef QueuePair_<StdMsg> QueuePair;
class GstThread
    : public BaseThread<StdMsg>
{
    public:
        GstThread();
        ~GstThread();
    private:
        VideoConfig* conf_;
        VideoSender* sender_;
        VideoReceiver* receiver_;

        std::string conf_str_;
        int main();

        GstThread(const GstThread&); //No Copy Constructor
        GstThread& operator=(const GstThread&); //No Assignment Operator
};


