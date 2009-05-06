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

void GstThread::handleMsg(MapMsg &msg)
{
    std::string s(msg.cmd());
    if(s == "audio_init")
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
            r["errmsg"] = e.msg_;
            queue_.push(r);
        }
    }
    else if(s == "stop")
    {
        stop(msg);
        stop_id = msg["id"];
    }
    else if(s == "start")
    {
        start(msg);
        play_id = msg["id"];
    }
    else if(s == "video_init")
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
            r["errmsg"] = e.msg_;
            queue_.push(r);
        }
    }
    else if (s == "rtp")
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
        LOG_INFO("video_init");
        const int SCREEN_NUM = 0;       
        const char *VIDEO_SINK = "xvimagesink";
        video_ = videofactory::buildVideoReceiver_(msg["address"], msg["codec"], msg["port"], SCREEN_NUM, VIDEO_SINK);
    }
    catch(Except e)
    {
        LOG_WARNING(e.msg_);
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
        const char *AUDIO_SINK = "jackaudiosink";
        const char *AUDIO_LOCATION = "";
        int audioBufferTime = audiofactory::AUDIO_BUFFER_USEC;
        if (msg["audio_buffer_usec"]) // take specified buffer time if present, otherwise use default
            audioBufferTime = msg["audio_buffer_usec"];

        audio_ = audiofactory::buildAudioReceiver_(msg["address"], msg["codec"], msg["port"], 
                AUDIO_SINK, AUDIO_LOCATION, audioBufferTime);
    }
    catch(Except e)
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
        LOG_INFO("video_init");
        std::string videoDevice, videoLocation;
        if(!msg["location"].empty())
            videoLocation = static_cast<std::string>(msg["location"]);
        if(!msg["device"].empty())
            videoDevice = static_cast<std::string>(msg["device"]);

        VideoSourceConfig config(msg["source"], msg["bitrate"], videoDevice, videoLocation, msg["deinterlace"]);
        video_ = videofactory::buildVideoSender_(config, msg["address"], msg["codec"], msg["port"]);
        if(ff[1])
            videoFirst = false;
        else
            videoFirst = true;

        ff[0] = boost::bind(tcpSendBuffer, msg["address"], static_cast<int>(msg["port"]) + ports::CAPS_OFFSET, videofactory::MSG_ID, _1);
    }
    catch(Except e)
    {
        LOG_WARNING(e.msg_);
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
        std::string audioDevice, audioLocation;
        
        if(!msg["location"].empty())
            audioLocation = static_cast<std::string>(msg["location"]);
        if(!msg["device"].empty())
            audioDevice = static_cast<std::string>(msg["device"]);

        AudioSourceConfig config(msg["source"], audioDevice, audioLocation, msg["channels"]);
        audio_ = audiofactory::buildAudioSender_(config, msg["address"], msg["codec"], msg["port"]);
         
        ff[1] = boost::bind(tcpSendBuffer,msg["address"], static_cast<int>(msg["port"]) + ports::CAPS_OFFSET, audiofactory::MSG_ID, _1);
    }
    catch(Except e)
    {
        LOG_WARNING(e.msg_);
        delete audio_;
        audio_ = 0;
        throw(e);
    }
}

bool GstReceiverThread::subHandleMsg(MapMsg &msg)
{
    std::string s(msg.cmd());
    if (s == "jitterbuffer")
    {
        updateJitterBuffer(msg);
        return true;    // no one else should handle this
    }
    else 
        return false;
}

void GstReceiverThread::updateJitterBuffer(MapMsg &msg)
{
    RtpReceiver::setLatency(msg["latency"]);
}

