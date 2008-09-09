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

//BaseModule args get deleted in ~BaseModule
// FIXME: sender and receiver should be in different processes, need to think about how
// we initiliaze VideoConfig, also should have AudioConfig. Are we ditching args_?
//


#define A_PORT 10010


GstThread::~GstThread()
{
    if(asender_)
        delete (asender_);
    if(vsender_)
        delete vsender_;
}


bool GstThread::audio_start(MapMsg& msg)
{
    if(asender_){
        delete (asender_);
        asender_ = 0;
    }
    if(msg["address"].type() == 's')
    {
        std::string addr;
        msg["address"].get(addr);
        AudioConfig config("audiotestsrc", 2, "vorbisenc", addr, A_PORT);
        asender_ = new AudioSender(config);
    }
    else{
        AudioConfig config("audiotestsrc", 2, "vorbisenc", get_host_ip(), A_PORT);
        asender_ = new AudioSender(config);
    }
    asender_->init();
    asender_->start();
    std::string caps_str;
    MapMsg caps;
    caps.insert( std::make_pair("command", "caps"));
    caps.insert( std::make_pair("caps_str", asender_->getCaps()));
    queue_.push(caps);

    return true;
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
            } else
            if(!s.compare("audio_start"))
            {
                audio_start(f);
            } else
            if(!s.compare("audio_stop"))
            {
                asender_->stop();
            }
            else{
                LOG_WARNING("Unknown Command.")
            }
        }
    }

    return 0;
}


#if 0
int GstThread::main()
{
    bool done = false;

    if(!conf_str_.empty())
    {
        conf_ = new VideoConfig(conf_str_);
        if(conf_)
            sender_ = new VideoSender(*conf_);
        if(!sender_)
        {
            GstMsg m(GstMsg::QUIT);
            queue_.push(m);
            done = true;
        }
    }
    while(!done)
    {
        GstMsg f = queue_.timed_pop(10000);
        switch(f.get_type())
        {
            case GstMsg::QUIT:
            {
                GstMsg ff(GstMsg::QUIT);
                queue_.push(ff);
                done = true;
                break;
            }
            case GstMsg::START:
            {
                sender_->start();
                break;
            }
            case GstMsg::INIT:
            {
                sender_->init();
                break;
            }
            case GstMsg::STOP:
            {
                sender_->stop();
                break;
            }

            default:
                break;
        }
    }

    return 0;
}


#endif

