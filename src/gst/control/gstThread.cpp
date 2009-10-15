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

#include "../engine/mediaBase.h"
#include "gstThread.h"
#include "gstSenderThread.h"
#include "../engine/playback.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "programOptions.h"

/**
 * The IPCP protocol is a home-made TCP ASCII protocol similar to telnet. 
 *
 * It is used to allow milhouse to be controlled by an other process, such as Python. (miville)
 *
 * The syntax is similar to a Python dict, but not quite. The command is either "audio_init" or
 * "video_init", followed by a colon (":") and the list of key-value pairs, separated by spaces. 
 * The UNIX endline character (13) is used to separate the messages. 
 *
 * Both the receiver thread and the sender thread receive an ASCII string that look similar, but with 
 * different arguments. They both accept the command "audio_init" and "video_init".
 *
 * A typical message to the sender and receiver thread is a string that looks like those examples below. 
 *
 * Messages to the SENDER :
 * 
 *   audio_init: codec="raw" port=10010 address="127.0.0.1" source="audiotestsrc" channels=2 ack="ok"
 *   video_init: codec="mpeg4" port=12007 address="127.0.0.1" bitrate=??? source="videotestsrc" device="" location=""
 *
 * Messages to the RECEIVER : 
 *
 *   audio_init: codec="raw" port=10010 address="127.0.0.1" channels=2  sink="jackaudiosink" device="" audio_buffer_usec=""
 *   video_init: codec="mpeg4" port=12007 address="127.0.0.1" bitrate=??? source=??? screen=0 sink="xvimagesink" deinterlace=???
 *
 * They also accept the message "start", "stop" and "rtp". They answer with a bunch of messages in the same fashion. 
 * Name, the "success" and "failure" messages. 
 */
void GstThread::stop(MapMsg& )
{ 
    playback::stop();
} 

void GstThread::start(MapMsg&)
{
    playback::start();
}


void GstSenderThread::start(MapMsg& )
{ 
    playback::start();

    hasPlayed_ = true;
} 

void GstThread::handleMsg(MapMsg &msg)
{
    if(msg() == "audio_init")
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
    else if(msg() == "stop")
    {
        stop(msg);
        stop_id = msg["id"];
    }
    else if(msg() == "start")
    {
        start(msg);
        play_id = msg["id"];
    }
    else if(msg() == "video_init")
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
    else if (msg() == "rtp")
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

                if(f())
                {
                    if(f() == "quit")
                    {
                        queue_.push(f);
                        done = true;
                        break;
                    }
                    
                    if(!subHandleMsg(f))
                        handleMsg(f);

                }
                f = queue_.timed_pop(1);
            }while(f());
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
}


void GstReceiverThread::setVideoDefaults(MapMsg &msg)
{
    static MapMsg defaults(ProgramOptions::defaultMapMsg());

    if (!msg["multicast-interface"])
        msg["multicast-interface"] = defaults["multicast-interface"];

    if (!msg["shared-video-id"])
        msg["shared-video-id"] = defaults["shared-video-id"];

    if(!msg["screen"])
        msg["screen"] = defaults["screen"];

    if(!msg["sink"])
        msg["sink"] = defaults["videosink"];

    if (!msg["address"])
        msg["address"] = defaults["address"];
}


void GstSenderThread::setVideoDefaults(MapMsg &msg)
{
    static MapMsg defaults(ProgramOptions::defaultMapMsg());

    if (!msg["device"]) 
        msg["device"] = defaults["videodevice"]; 

    if (!msg["location"]) 
        msg["location"] = defaults["videolocation"]; 

    if (!msg["quality"])
        msg["quality"] = defaults["videoquality"];

    // Only use quality if we're using theora or no bitrate has been set
    if (msg["quality"])
        if (msg["bitrate"])
        {
            LOG_WARNING("Ignoring quality setting for " << msg["codec"]);
            msg["quality"] = defaults["videoquality"];
        }

    // If quality is != 0, then we know that we're using theora from the previous check
    if (!msg["bitrate"])
    {
        if (!msg["quality"])
            msg["bitrate"] = defaults["videobitrate"]; // quality and bitrate are mutually exclusive
    }

    if (!msg["camera-number"])
        msg["camera-number"] = defaults["camera-number"];

    if (!msg["address"])
        msg["address"] = defaults["address"];
}


void GstReceiverThread::setAudioDefaults(MapMsg &msg)
{
    static MapMsg defaults(ProgramOptions::defaultMapMsg());

    if (!msg["numchannels"]) 
        msg["numchannels"] = defaults["numchannels"];

    if (!msg["device"])
        msg["device"] = defaults["audiodevice"];

    if (!msg["audio-buffer-usec"])
        msg["audio-buffer-usec"] = defaults["audio-buffer-usec"];

    if (!msg["sink"])
        msg["sink"] = defaults["jackaudiosink"];

    if (!msg["multicast-interface"])
        msg["multicast-interface"] = defaults["multicast-interface"]; 

    if (!msg["address"])
        msg["address"] = defaults["address"];

    if (!msg["jack-client-name"])
        msg["jack-client-name"] = defaults["jack-client-name"];
}


void GstSenderThread::setAudioDefaults(MapMsg &msg)
{
    static MapMsg defaults(ProgramOptions::defaultMapMsg());

    if (!msg["numchannels"]) 
        msg["numchannels"] = defaults["numchannels"];

    if (!msg["device"])
        msg["device"] = defaults["audiodevice"]; 

    if (!msg["location"])
        msg["location"] = defaults["audiolocation"];

    if (!msg["address"])
        msg["address"] = defaults["address"];

    if (!msg["jack-client-name"])
        msg["jack-client-name"] = defaults["jack-client-name"];
}


void GstReceiverThread::video_init(MapMsg& msg)
{
    video_.reset();

    try
    {
        setVideoDefaults(msg);
        video_ = videofactory::buildVideoReceiver(msg);
    }
    catch(ErrorExcept e)
    {
        LOG_WARNING(e.what());
        throw(e);
    }
}


void GstReceiverThread::audio_init(MapMsg& msg)
{
    audio_.reset();

    try
    {
        setAudioDefaults(msg);
        audio_ = audiofactory::buildAudioReceiver(msg);
    }
    catch(ErrorExcept e)
    {
        throw(e);
    }
}


GstSenderThread::~GstSenderThread()
{
}


void GstSenderThread::video_init(MapMsg& msg)
{
    video_.reset();

    try
    {
        setVideoDefaults(msg);
        video_ = videofactory::buildVideoSender(msg);
    }
    catch(ErrorExcept e)
    {
        LOG_ERROR(e.what());
        throw(e);
    }
}


void GstSenderThread::audio_init(MapMsg& msg)
{
    audio_.reset();

    try
    {
        setAudioDefaults(msg);
        audio_ = audiofactory::buildAudioSender(msg);
    }
    catch(ErrorExcept e)
    {
        LOG_ERROR(e.what());
        throw(e);
    }
}

/// Receiver specific messages
bool GstReceiverThread::subHandleMsg(MapMsg &msg)
{
    if (msg() == "jitterbuffer")
    {
        updateJitterBuffer(msg);
        return true;    // no one else should handle this
    }

    return false;
}

void GstReceiverThread::updateJitterBuffer(MapMsg &msg)
{
    RtpReceiver::setLatency(msg["jitterbuffer"]);
}

