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

OscMessage::OscMessage(const char *p, const char *t, lo_arg ** v, int c, void *d) : path(p), types(t), argc(c), data(d)
{
	for (int i = 0; i < c; i++)
	{
		args.push_back(LoArgs(t, i, v[i]));
	}

}

OscThread::OscThread()
{
	args.clear();
	args.push_back(new StringArg(&port_,"osc",'\0',"osc port", "port num"));

}


int OscThread::generic_handler_static(const char *path, const char *types,
                                      lo_arg ** argv, int argc, void *data, void *user_data)
{
	OscThread *t = static_cast < OscThread * >(user_data);
	return (t->generic_handler(path, types, argv, argc, data));
}

int OscThread::generic_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data)
{
	queue.push(OscMessage(path, types, argv, argc, data));
	if (queue_map.find(path) != queue_map.end())
		queue_map[path].push(OscMessage(path, types, argv, argc, data));
	return 0;
}

int OscThread::main()
{
//      static Message r(message::ok);
	if(!port_)
		return -1;

	lo_server_thread st = lo_server_thread_new(port_, liblo_error);

	lo_server_thread_add_method(st, NULL, NULL, generic_handler_static, this);

	lo_server_thread_start(st);

	while (1)
	{
		OscMessage msg = queue.copy_timed_pop(20);
		if (!msg.path.empty()) {
			send(msg);
		}

	}
	return 0;
}

void OscThread::send(OscMessage & osc)
{
	lo_address t = lo_address_new(NULL, "7771");
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
