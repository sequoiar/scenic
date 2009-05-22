/* gstThread.cpp
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

#include "util.h"
#include <glib.h>

#include "engine/mediaBase.h"
#include "gstThread.h"
#include "gstSenderThread.h"
#include "engine/playback.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>

void GstThread::stop(MapMsg& ){ playback::stop();} 
void GstThread::start(MapMsg&)
{
    playback::start();
}


void GstSenderThread::start(MapMsg& )
{ 
    playback::start();
    if(!hasPlayed_)
    {
        if(videoFirst)
        {
            if(ff[0])
                ff[0](video_->getCaps());
            if(ff[1])
                ff[1](audio_->getCaps());
        }
        else
        {
            if(ff[1])
                ff[1](audio_->getCaps());
            if(ff[0])
                ff[0](video_->getCaps());
        }
    }

    hasPlayed_ = true;
} 

void GstThread::handleMsg(MapMsg &msg)
{
    if(msg.cmd() == "audio_init")
    {
        try
        {
            audio_init(msg);
            MapMsg r("success");
            r["id"] = msg["id"];
            queue_.push(r);
        }
        catch(Except e)
        {
            MapMsg r("failure");
            r["id"] = msg["id"];
            r["errmsg"] = e.what();
            queue_.push(r);
        }
        
    }
    else if(msg.cmd() == "stop")
    {
        stop(msg);
        stop_id = msg["id"];
    }
    else if(msg.cmd() == "start")
    {
        start(msg);
        play_id = msg["id"];
    }
    else if(msg.cmd() == "video_init")
    {
        try
        {
            video_init(msg);
            MapMsg r("success");
            r["id"] = msg["id"];
            queue_.push(r);
        }
        catch(Except e)
        {
            MapMsg r("failure");
            r["id"] = msg["id"];
            r["errmsg"] = e.what();
            queue_.push(r);
        }
    }
    else if (msg.cmd() == "rtp")
        queue_.push(msg);
    else
        LOG_WARNING("Unknown Command.");
}

void GstThread::main()
{
    bool done = false;
    while(!done)
    {
        if(g_main_context_iteration(NULL, FALSE))
            continue;

        if(queue_.ready())
        {
            MapMsg f = queue_.timed_pop(1);

            do
            {
                if(play_id && playback::isPlaying())
                {
                    MapMsg r("success");
                    r["id"] = play_id;
                    queue_.push(r);
                    play_id = 0;
                }
                if(stop_id && !playback::isPlaying())
                {
                    MapMsg r("success");
                    r["id"] = stop_id;
                    queue_.push(r);
                    stop_id = 0;
                }

                if(f.cmd())
                {
                    if(f.cmd() == "quit")
                    {
                        queue_.push(f);
                        done = true;
                        break;
                    }
                    
                    if(!subHandleMsg(f))
                        handleMsg(f);

                }
                f = queue_.timed_pop(1);
            }while(f.cmd());
        }
        usleep(MILLISEC_WAIT*1000);
    }

}


#include "util.h"

#include "gstReceiverThread.h"
#include "gstSenderThread.h"

#include "gst/videoFactory.h"
#include "gst/audioFactory.h"

GstReceiverThread::~GstReceiverThread()
{
    delete video_;
    delete audio_;
}


void GstReceiverThread::video_init(MapMsg& msg)
{
    delete video_;
    video_ = 0;
 
    try
    {
        if(!msg["screen"])
            msg["screen"] = 0;
        if(!msg["sink"])
            msg["sink"] = "xvimagesink";

        video_ = videofactory::buildVideoReceiver_(msg["address"], msg["codec"], msg["port"], msg["screen"], msg["sink"], msg["deinterlace"]);
    }
    catch(ErrorExcept e)
    {
        LOG_WARNING(e.what());
        delete video_;
        video_ = 0;
        throw(e);
    }
}


void GstReceiverThread::audio_init(MapMsg& msg)
{
    delete audio_;
    audio_ = 0;

    try
    {
        int audioBufferTime = audiofactory::AUDIO_BUFFER_USEC;
        if(!msg["sink"])
            msg["sink"] = "jackaudiosink";

        if(!msg["device"])
            msg["device"] = "";
        
        if (msg["audio_buffer_usec"]) // take specified buffer time if present, otherwise use default
            audioBufferTime = msg["audio_buffer_usec"];

        audio_ = audiofactory::buildAudioReceiver_(msg["address"], msg["codec"], msg["port"], 
                msg["sink"], msg["device"], audioBufferTime);
    }
    catch(ErrorExcept e)
    {
        delete audio_;
        audio_ = 0;
        throw(e);
    }
}


GstSenderThread::~GstSenderThread()
{
    delete video_;
    delete audio_;
}


void GstSenderThread::video_init(MapMsg& msg)
{
    delete video_;
    video_ = 0;
    try
    {
        if(!msg["location"])
            msg["location"] = "";
        if(!msg["device"])
            msg["device"] = "";

        VideoSourceConfig config(msg["source"], msg["bitrate"], msg["device"], msg["location"]);
        video_ = videofactory::buildVideoSender_(config, msg["address"], msg["codec"], msg["port"]);
        if(ff[1])
            videoFirst = false;
        else
            videoFirst = true;

        ff[0] = boost::bind(tcpSendBuffer, msg["address"], msg["port"] + ports::CAPS_OFFSET, videofactory::MSG_ID, _1);
    }
    catch(ErrorExcept e)
    {
        LOG_WARNING(e.what());
        delete video_;
        video_ = 0;
        throw(e);
    }
}


void GstSenderThread::audio_init(MapMsg& msg)
{
    delete audio_;
    audio_ = 0;
    try
    {
        LOG_INFO("audio_init");
        
        if(!msg["location"])
            msg["location"] = "";
        if(!msg["device"])
            msg["device"] = "";

        AudioSourceConfig config(msg["source"], msg["device"], msg["location"], msg["channels"]);
        audio_ = audiofactory::buildAudioSender_(config, msg["address"], msg["codec"], msg["port"]);
         
        ff[1] = boost::bind(tcpSendBuffer, msg["address"], msg["port"] + ports::CAPS_OFFSET, audiofactory::MSG_ID, _1);
    }
    catch(ErrorExcept e)
    {
        LOG_ERROR(e.what());
        delete audio_;
        audio_ = 0;
        throw(e);
    }
}

bool GstReceiverThread::subHandleMsg(MapMsg &msg)
{
    if (msg.cmd() == "jitterbuffer")
    {
        updateJitterBuffer(msg);
        return true;    // no one else should handle this
    }

    return false;
}

void GstReceiverThread::updateJitterBuffer(MapMsg &msg)
{
    RtpReceiver::setLatency(msg["latency_msec"]);
}

