/* GstSenderThread.cpp 
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
 */

#include "gstSenderThread.h"
#include "engine/audioSender.h"
#include "engine/videoSender.h"
#include "engine/playback.h"

GstSenderThread::~GstSenderThread()
{
    delete video_;
    delete audio_;
}

bool GstSenderThread::video_start(MapMsg& msg)
{
    delete video_;
    video_ = 0;

    try
    {
        //VideoSourceConfig config("dv1394src");
        SenderConfig rConfig(msg["codec"], msg["address"], msg["port"]);

        if(msg["location"].empty())
        {
            VideoSourceConfig config(msg["source"]);
            video_ =  new VideoSender(config, rConfig);
        }
        else
        {
            VideoSourceConfig config(msg["source"], std::string(msg["location"]));
            video_ =  new VideoSender(config, rConfig);
        }
        video_->init();
        playback::start();
        return true;
    }
    catch(Except e)
    {
        LOG_WARNING(e.msg_);
        delete video_;
        video_ = 0;
        return false;
    }
}


bool GstSenderThread::audio_start(MapMsg& msg)
{
    delete audio_;
    audio_ = 0;
    try
    {
        AudioSender* asender;

        SenderConfig rConfig(msg["codec"], msg["address"], msg["port"]);
        if(msg["location"].empty())
        {
            AudioSourceConfig config(msg["source"], msg["channels"]);
            audio_ = asender = new AudioSender(config, rConfig);
        }
        else
        {
            AudioSourceConfig config(msg["source"], msg["location"], msg["channels"]);
            audio_ = asender = new AudioSender(config, rConfig);
        }
        audio_->init();
        playback::start();

        //Build Caps Msg
        MapMsg caps("caps");
        caps["caps_str"] = asender->getCaps();
        //Forward to tcp
        queue_.push(caps);
        return true;
    }
    catch(Except e)
    {
        LOG_WARNING(e.msg_);
        delete audio_;
        audio_ = 0;
        return false;
    }
}


