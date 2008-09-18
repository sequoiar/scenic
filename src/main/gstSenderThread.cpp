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
#include "hostIP.h"

#include "gst/audioSender.h"
#include "gst/videoSender.h"



GstSenderThread::~GstSenderThread()
{
    if(asender_)
        delete asender_;
    if(vsender_)
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
    if(vsender_){
        delete (vsender_);
        vsender_ = 0;
    }
    std::string addr;
    if(msg["address"].type() == 's')
        msg["address"].get(addr);
    else
        addr = get_host_ip();
    GET_OR_RETURN(msg, "port", int, port);
    VideoConfig config("videotestsrc");
    SenderConfig rConfig("h264", addr, port);
    if(!config.sanityCheck())
        return false;
    vsender_ = new VideoSender(config, rConfig);
    if(vsender_)
    {
        vsender_->init();
        vsender_->start();
        return true;
    }
    else
        return false;
}


bool GstSenderThread::audio_start(MapMsg& msg)
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
    GET_OR_RETURN(msg, "port", int, port);
    AudioConfig config("audiotestsrc", 2);
    SenderConfig rConfig("vorbis", addr, port);
    if(!config.sanityCheck())
        return false;
    asender_ = new AudioSender(config, rConfig);
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


