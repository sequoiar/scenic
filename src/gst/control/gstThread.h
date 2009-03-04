/* gstThread.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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
 *
 */

#ifndef __GST_THREAD__
#define __GST_THREAD__
#include "msgThread.h"

/// MapMsg handler thread base class that calls GST media functionality
class GstThread
    : public MsgThread
{
    protected:
        GstThread(){}
        /// incomming audio_start request 
        virtual bool audio_start(MapMsg& msg) = 0;
        /// incomming video_start request 
        virtual bool video_start(MapMsg& msg) = 0;
        /// incomming stop request 
        void stop(MapMsg& ); 
        /// incomming start request 
        virtual void start(MapMsg& ); 



    private:
        int main();

        /// No Copy Constructor 
        GstThread(const GstThread&); 
        /// No Assignment Operator 
        GstThread& operator=(const GstThread&); 
};

#endif // __GST_THREAD__

