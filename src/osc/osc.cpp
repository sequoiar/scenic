/*
   // Copyright 2008 Koya Charles & Tristan Matthews
   //
   // This file is part of [propulse]ART.
   //
   // [propulse]ART is free software: you can redistribute it and/or modify
   // it under the terms of the GNU General Public License as published by
   // the Free Software Foundation, either version 3 of the License, or
   // (at your option) any later version.
   //
   // [propulse]ART is distributed in the hope that it will be useful,
   // but WITHOUT ANY WARRANTY; without even the implied warranty of
   // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   // GNU General Public License for more details.
   //
   // You should have received a copy of the GNU General Public License
   // along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *
 *
 *
 *
 *
 */

#include <iostream>
#include <string.h>

#include <lo/lo.h>
#include "osc.h"


OscThread::OscThread()
    : local_port_("7770"), remote_port_(), remote_host_(), arg_local_port_(0), running_(false)
{
    args_.clear();
    //    local_port_ = remote_port_ = remote_host_ = 0;
    //args_.push_back(new StringArg(arg_local_port_, "oscLocal", 'p', "local osc port",
    //                            "port num"));

    //arg_local_port_ = reinterpret_cast<const char *>("7770");
    //args.push_back(new StringArg(&remote_port_, "oscRemote", '\0', "remote osc port",
    //                             "port num"));
    //args.push_back(new StringArg(&remote_host_, "oscRemoteHost", '\0', "host", "host address"));
}


int OscThread::generic_handler_static(const char *path, const char *types, lo_arg ** argv,
                                      int argc, void *data,
                                      void *user_data)
{
    OscThread *t = static_cast < OscThread * >(user_data);
    return (t->generic_handler(path, types, argv, argc, data));
}


int OscThread::generic_handler(const char *path, const char *types, lo_arg ** argv, int argc,
                               void *data)
{
    queue_.push(OscMessage(path, types, argv, argc, data));
    return 0;
}


int OscThread::main()
{
    if(local_port_.empty())
        return -1;
    lo_server_thread st = lo_server_thread_new(local_port_.c_str(), liblo_error);

    lo_server_thread_add_method(st, NULL, NULL, generic_handler_static, this);

    lo_server_thread_start(st);

    running_ = true;
    while (running_)
    {
        OscMessage msg = queue_.timed_pop(10000);

        if (msg.pathIsSet()) {
            if(msg.path().compare(0,5,"/quit"))
                return 0;
            else
                send(msg);
        }
    }
    return 0;
}


bool OscThread::ready()
{
    if(local_port_.empty() )
        return false;
    if(remote_host_.empty())
        remote_host_ = "127.0.0.1";
    return true;
}


bool OscThread::send(OscMessage & osc)
{
    if(remote_port_.empty())
        return false;
    lo_address t = lo_address_new(remote_host_.c_str(), remote_port_.c_str());
    lo_message m = lo_message_new();

    osc.init_msg(&m);

    lo_send_message(t, osc.path().c_str(), m);

    return true;
}


