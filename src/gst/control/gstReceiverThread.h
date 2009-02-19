/* gstReceiverThread.h
 * Copyright (C) 2009 Société des arts technologiques (SAT)
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

#ifndef _GST_RECEIVER_THREAD_H_
#define _GST_RECEIVER_THREAD_H_

#include <glib.h>
#include <iostream>
#include "gstThread.h"
class ReceiverBase;

/**MapMsg handler thread that calls GST media functionality
 *
 *
 \msc 
  miville,gst;

    miville->gst [label="audio_init:"];
    gst->miville [label="audio_init: ack=ok"];
    miville->gst [label="video_init:"];
    gst->miville [label="video_init: ack=ok"];
    miville->gst [label="start:"];
    gst->miville [label="start: ack=ok"];
    --- [label="receiving stream(s)"];
    miville->gst [label="stop"];
    gst->miville [label="stop: ack=ok"];
    --- [label="stopped receiving stream(s)"];
 \endmsc
*/
class GstReceiverThread
    : public GstThread
{
    public:
        GstReceiverThread()
            : video_(0), audio_(0) {}
        ~GstReceiverThread();
    private:
        /// incomming audio_start request 
        bool audio_start(MapMsg& msg);
        /// incomming video_start request 
        bool video_start(MapMsg& msg);
        
        ReceiverBase* video_;
        ReceiverBase* audio_;

        /// No Copy Constructor 
        GstReceiverThread(const GstReceiverThread&); 
        /// No Assignment Operator 
        GstReceiverThread& operator=(const GstReceiverThread&); 
};


#endif // _GST_RECEIVER_THREAD_H_

