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

//BaseModule args get deleted in ~BaseModule
// FIXME: sender and receiver should be in different processes, need to think about how 
// we initiliaze VideoConfig, also should have AudioConfig. Are we ditching args_?
// 

GstThread::GstThread()
    : conf_(0), sender_(0), receiver_(0), conf_str_("dv1394src")
{
    args_.clear();
#if 0
    conf_str = 0;
    args_.push_back(new StringArg(&conf_str, "sender", 's', "video", "try videotestsrc"));
#endif
}


GstThread::~GstThread()
{
    delete sender_;
    delete conf_;
}


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
            BaseMessage m(BaseMessage::QUIT);
            queue_.push(m);
            done = true;
        }
    }
    while(!done)
    {
        BaseMessage f = queue_.timed_pop(10000);
        switch(f.get_type())
        {
            case BaseMessage::QUIT:
            {
                BaseMessage f(BaseMessage::QUIT);
                queue_.push(f);
                done = true;
                break;
            }
            case BaseMessage::START:
            {
                sender_->start();
                break;
            }
            case BaseMessage::INIT:
            {
                sender_->init();
                break;
            }
            case BaseMessage::STOP:
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


