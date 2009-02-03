/* GTHREAD-QUEUE-PAIR - Library of GstReceiverThread Queue Routines for GLIB
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

#include "util.h"

#include "gstReceiverThread.h"
#include "gstSenderThread.h"

#include "gst/videoFactoryInternal.h"
#include "gst/audioFactoryInternal.h"

GstReceiverThread::~GstReceiverThread()
{
    delete video_;
    delete audio_;
}

bool GstReceiverThread::video_start(MapMsg& msg)
{
    delete video_;
    video_ = 0;

    try
    {
        video_ = videofactory::buildVideoReceiver_(get_host_ip(),msg["codec"].c_str().c_str(),msg["port"],0,"xvimagesink");
        video_->init();
        playback::start();
//        queue_.push(MapMsg("video_started"));
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


bool GstReceiverThread::audio_start(MapMsg& msg)
{
    delete audio_;
    audio_ = 0;

    try
    {
        audio_ = audiofactory::buildAudioReceiver_(get_host_ip(),msg["codec"].c_str().c_str(),msg["port"]);
        audio_->init();
        playback::start();
//        queue_.push(MapMsg("audio_started"));
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
            video_ = videofactory::buildVideoSender_(config,msg["address"].c_str().c_str(),msg["codec"].c_str().c_str(),msg["port"]);
        }
        else
        {
            VideoSourceConfig config(msg["source"], std::string(msg["location"]));
            video_ = videofactory::buildVideoSender_(config,msg["address"].c_str().c_str(),msg["codec"].c_str().c_str(),msg["port"]);
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
            audio_ = asender = audiofactory::buildAudioSender_(config,msg["address"].c_str().c_str(),msg["codec"].c_str().c_str(),msg["port"]);
        }
        else
        {
            AudioSourceConfig config(msg["source"], msg["location"], msg["channels"]);
            audio_ = asender = audiofactory::buildAudioSender_(config,msg["address"].c_str().c_str(),msg["codec"].c_str().c_str(),msg["port"]);
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


