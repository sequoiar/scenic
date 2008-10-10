/* GTHREAD-QUEUE-PAIR - Library of GstThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redisttribute it and/or
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
#include "gstThread.h"
#include "logWriter.h"
#include <iostream>
int GstThread::main()
{
    bool done = false;
    bool flipflop = false;
    while(!done)
    {
        if(g_main_context_iteration(NULL,FALSE)) 
            continue;

        std::cout << (flipflop ? "-\r" : " \r");
        flipflop = !flipflop;
        std::cout.flush();
        updateState();
        MapMsg f = queue_.timed_pop(100000);
        
        if(f["command"].type() == 's')
        {
            std::string s;
            f["command"].get(s);
            LOG_DEBUG(s);
            
            if(s == "quit")
            {
                queue_.push(f);
                done = true;
            }
            else if(s == "audio_start")
            {
                audio_start(f);
            }
            else if(s == "audio_stop")
            {
                audio_stop(f);
            }
            else if(s == "video_start")
            {
                video_start(f);
            }
            else if(s == "video_stop")
            {
                video_stop(f);
            }
            else if(s == "levels")
            {
                queue_.push(f);
            }
            else
                LOG_WARNING("Unknown Command.");
        }
    }

    return 0;
}


void GstThread::updateState()
{

}

GstThread::~GstThread()
{
    delete audio_;
    delete video_;
}


bool GstThread::video_stop(MapMsg& /*msg*/)
{
    if(!video_)
        return false;
    video_->stop();
    return true;
}


bool GstThread::audio_stop(MapMsg& /*msg*/)
{
    if(!video_)
        return false;
    video_->stop();
    return true;
}

