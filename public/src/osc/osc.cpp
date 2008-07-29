//
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
//

/** \file
 *
 *
 *
 *
 *
 */

#include <iostream>

#include <lo/lo.h>
#include "osc.h"

OscMessage::OscMessage(const char *p, const char *t, lo_arg ** v, int c, void *d)
    : path(p), types(t), args(), argc(c), data(d)
{
    for (int i = 0; i < c; i++)
        args.push_back(LoArgs(t, i, v[i]));
}


OscMessage::OscMessage(const OscMessage& in)
    : path(in.path), types(in.types), args(in.args), argc(in.argc), data(in.data)
{
    // empty
}


OscMessage& OscMessage::operator=(const OscMessage& in)
{
    if (this == &in)
        return *this; // gracefully handle self-assignment

    path = in.path;
    types = in.types;
    args = in.args;
    argc = in.argc;
    data = in.data;

    return *this;
}


OscThread::OscThread()
    : local_port_(0),remote_port_(0),remote_host_(),running(false)
{
    args.clear();
    local_port_ = remote_port_ = remote_host_ = 0;
    args.push_back(new StringArg(&local_port_,"oscLocal",'\0',"local osc port","port num"));
    args.push_back(new StringArg(&remote_port_,"oscRemote",'\0',"remote osc port","port num"));
    args.push_back(new StringArg(&remote_host_,"oscRemoteHost",'\0',"host","host address"));
}


int OscThread::generic_handler_static(const char *path, const char *types,lo_arg ** argv,
                                      int argc, void *data, void *user_data)
{
    OscThread *t = static_cast < OscThread * >(user_data);
    return (t->generic_handler(path, types, argv, argc, data));
}


int OscThread::generic_handler(const char *path, const char *types, lo_arg ** argv,int argc,
                               void *data)
{
    queue.push(OscMessage(path, types, argv, argc, data));
    return 0;
}


int OscThread::main()
{
    if(!local_port_)
        return -1;
    lo_server_thread st = lo_server_thread_new(local_port_, liblo_error);

    lo_server_thread_add_method(st, NULL, NULL, generic_handler_static, this);

    lo_server_thread_start(st);

    running = true;
    while (running)
    {
        OscMessage msg = queue.timed_pop(10000);

        if (!msg.path.empty()) {
            if(!msg.path.compare("/quit"))
                return 0;
            else
                send(msg);
        }
    }
    return 0;
}


void OscThread::send(OscMessage & osc)
{
    if(!remote_port_)
        return;
    lo_address t = lo_address_new(remote_host_, remote_port_);
    lo_message m = lo_message_new();

    for (OscArgs::iterator it = osc.args.begin(); it != osc.args.end(); ++it)
    {
        switch ((char) it->type)
        {
        case 's':
        {
            lo_message_add_string(m, it->s.c_str());
            break;
        }
        case 'i':
        {
            lo_message_add_int32(m, it->i);
            break;
        }
        }
    }

    lo_send_message(t, osc.path.c_str(), m);
}


