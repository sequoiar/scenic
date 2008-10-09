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


GstSenderThread::~GstSenderThread()
{
    delete asender_;
    delete vsender_;
}


bool GstSenderThread::video_stop(MapMsg& /*msg*/)
{
    if(vsender_)
        vsender_->stop();
    else
        return false;
    return true;
}


bool GstSenderThread::audio_stop(MapMsg& /*msg*/)
{
    if(asender_)
        asender_->stop();
    else
        return false;
    return true;
}


bool GstSenderThread::video_start(MapMsg& msg)
{
    delete vsender_;
    vsender_ = 0;
    
    try
    {
        VideoConfig config("videotestsrc");
        SenderConfig rConfig(msg["codec"], msg["address"] , msg["port"]);
        if(!config.sanityCheck())
            return false;

        vsender_ = new VideoSender(config, rConfig);

        vsender_->init();
        vsender_->start();

        return true;
    }
    catch(Except e)
    {
        LOG_WARNING(e.msg_);
        delete vsender_;
        vsender_ = 0;
        return false;
    }
}

bool GstSenderThread::audio_start(MapMsg& msg)
{
    delete asender_;
    asender_ = 0;

    sleep(5);
    try
    {
        AudioConfig config("audiotestsrc", 2);
        SenderConfig rConfig("vorbis", msg["address"], msg["port"]);
            
        asender_ = new AudioSender(config, rConfig);

        asender_->init();
        asender_->start();

        //Build Caps Msg 
        MapMsg caps("caps");
        caps["caps_str"] = asender_->getCaps();

        //Forward to tcp
        queue_.push(caps);

        return true;
    }
    catch(Except e)
    {
        LOG_WARNING(e.msg_);
        delete asender_;
        asender_ = 0;
        return false;
    }
}


