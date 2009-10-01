/* gstSenderThread.h
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

#ifndef _GST_SENDER_THREAD_H_
#define _GST_SENDER_THREAD_H_

#include "gstThread.h"
#include <boost/shared_ptr.hpp>

class SenderBase;
/// MapMsg handler thread that calls GST media functionality
class GstSenderThread
    : public GstThread
{
    public:
        GstSenderThread()
            : video_(), audio_() {}
        ~GstSenderThread();
        virtual void start(MapMsg& ); 
    private:
        /// incomming audio_start request 
        void audio_init(MapMsg& msg);
        /// incomming video_start request 
        void video_init(MapMsg& msg);

        boost::shared_ptr<SenderBase> video_;
        boost::shared_ptr<SenderBase> audio_;

        /// No Copy Constructor 
        GstSenderThread(const GstSenderThread&); 
        /// No Assignment Operator 
        GstSenderThread& operator=(const GstSenderThread&); 
};

#endif // _GST_SENDER_THREAD_H_

