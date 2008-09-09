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
#include "hostIP.h"
#include "logWriter.h"

// FIXME: sender and receiver should be in different processes, need to think about how
//


#define A_PORT 10010
#define V_PORT 10110


GstThread::~GstThread()
{
    if(asender_)
        delete (asender_);
    if(vsender_)
        delete vsender_;
}


int GstThread::main()
{
    bool done = false;

    while(!done)
    {
        MapMsg f = queue_.timed_pop(10000);
        if(f["command"].type() == 's')
        {
            std::string s;
            f["command"].get(s);

            if(!s.compare("quit"))
            {
                queue_.push(f);
                done = true;
            }
            else if(!s.compare("audio_start"))
            {
                audio_start(f);
            }
            else if(!s.compare("audio_stop"))
            {
                if(asender_)
                    asender_->stop();
            }
            else if(!s.compare("video_start"))
            {
                video_start(f);
            }
            else if(!s.compare("video_stop"))
            {
                if(vsender_)
                    vsender_->stop();
            }
            else{
                LOG_WARNING("Unknown Command.");
            }
        }
    }

    return 0;
}


bool GstThread::video_start(MapMsg& msg)
{
    if(vsender_){
        delete (vsender_);
        vsender_ = 0;
    }
    std::string addr;
    if(msg["address"].type() == 's')
        msg["address"].get(addr);
    else
        addr = get_host_ip();
    VideoConfig config("videotestsrc", "h264", addr, V_PORT);
    vsender_ = new VideoSender(config);
    if(vsender_)
    {
        vsender_->init();
        vsender_->start();
        return true;
    }
    else
        return false;
}


bool GstThread::audio_start(MapMsg& msg)
{
    if(asender_){
        delete (asender_);
        asender_ = 0;
    }
    std::string addr;
    if(msg["address"].type() == 's')
        msg["address"].get(addr);
    else
        addr = get_host_ip();
    AudioConfig config("audiotestsrc", 2, "vorbisenc", addr, A_PORT);
    asender_ = new AudioSender(config);
    if(asender_)
    {
    asender_->init();
    asender_->start();
    std::string caps_str;
    MapMsg caps;
    caps.insert( std::make_pair("command", "caps"));
    caps.insert( std::make_pair("caps_str", asender_->getCaps()));
    queue_.push(caps);
        return true;
    }
    else
        return false;
}


