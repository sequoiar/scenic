/* gstThread.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

#include "gstThread.h"
#include "logWriter.h"
#include "engine/playback.h"
#include <iostream>

        void GstThread::audio_stop(MapMsg& ){ playback::stop();} 
        void GstThread::video_stop(MapMsg& ){ playback::stop();} 
int GstThread::main()
{
    bool done = false;
    bool flipflop = false;
    while(!done)
    {
        if(g_main_context_iteration(NULL, FALSE))
            continue;
        std::cout << (flipflop ? "-\r" : " \r");
        flipflop = !flipflop;
        std::cout.flush();
        MapMsg f = queue_.timed_pop(100000);

        if(!f["command"].empty())
        {
            std::string s;
            f["command"].get(s);

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

