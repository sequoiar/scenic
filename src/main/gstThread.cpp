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
GstThread::GstThread()
    : conf(), sender(), receiver(), conf_str_("videotestsrc")
{
    args_.clear();
#if 0
    conf_str = 0;
    args_.push_back(new StringArg(&conf_str, "sender", 's', "video", "try videotestsrc"));
#endif
}


int GstThread::main()
{
    bool quit = false;

    if(!conf_str_.empty())
    {
        conf = new VideoConfig(conf_str_);
        if(conf)
            sender = new VideoSender(*conf);

        if(!sender)
        {
            BaseMessage m(BaseMessage::QUIT);
            queue_.push(m);
            quit = true;
        }
    }

    while(!quit)
    {
        BaseMessage f = queue_.timed_pop(10000);
        switch(f.get_type())
        {
            case BaseMessage::QUIT:
            {
                BaseMessage f(BaseMessage::QUIT);
                queue_.push(f);
                quit = true;
                break;
            }
            case BaseMessage::START:
            {
                sender->start();
                break;
            }
            case BaseMessage::INIT:
            {
                sender->init();
                break;
            }
            case BaseMessage::STOP:
            {
                sender->stop();
                break;
            }

            default:
                break;
        }
    }


    return 0;
}


