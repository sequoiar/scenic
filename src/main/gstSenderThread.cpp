/* GTHREAD-QUEUE-PAIR - Library of GstSenderThread Queue Routines for GLIB
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
#include "gstSenderThread.h"

#include "gst/audioSender.h"
#include "gst/videoSender.h"



bool GstSenderThread::video_start(MapMsg& msg)
{
    delete video_;
    video_ = 0;
    
    try
    {
        //VideoConfig config("dv1394src");
        VideoConfig config("videotestsrc");
        SenderConfig rConfig(msg["codec"], msg["address"] , msg["port"]);
        video_ =  new VideoSender(config, rConfig);
        video_->init();
        video_->start();
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
        AudioConfig config("audiotestsrc", 2);
        SenderConfig rConfig("vorbis", msg["address"], msg["port"]);
        AudioSender* asender;
        audio_ = asender = new AudioSender(config, rConfig);
        audio_->init();
        audio_->start();

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


