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
#include "gstReceiverThread.h"

#include "engine/audioReceiver.h"
#include "engine/videoReceiver.h"
#include "engine/playback.h"
#include "hostIP.h"

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
        VideoSinkConfig config("xvimagesink");
        ReceiverConfig rConfig(msg["codec"], get_host_ip(), msg["port"], "");
        video_ = new VideoReceiver(config, rConfig);
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
        AudioSinkConfig config("jackaudiosink");
        ReceiverConfig rConfig(msg["codec"], get_host_ip(), msg["port"], msg["caps"]);
        audio_ = new AudioReceiver(config, rConfig);
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


