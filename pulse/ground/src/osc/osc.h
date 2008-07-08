// headerGPL.c
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
 *      Osc Message and Thread
 *
 *      Detailed description here.
 *      Continues here.
 *      And more.
 *      And more.
 */

#ifndef __OSC_THREAD_H__
#define __OSC_THREAD_H__

#include "gutil/baseThread.h"
#include <lo/lo.h>
#include <string>
#include <vector>
#include <iostream>

class LoArgs;
typedef std::vector < LoArgs > OscArgs;
class OscMessage
{
public:
	OscMessage(const char *p, const char *t, lo_arg ** v, int c, void *d);
	OscMessage()
	{
	}
	std::string path, types;
	OscArgs args;
	int argc;
	void *data;

};

typedef QueuePair_ < OscMessage > QueuePairOfOscMessage;
class OscThread : public BaseThread < OscMessage >
{
	int main();

	static int generic_handler_static(const char *path, const char *types,
	                                  lo_arg ** argv, int argc, void *data, void *user_data);

	int generic_handler(const char *path, const char *types, lo_arg ** argv, int argc, void *data);

	static void liblo_error(int num, const char *msg, const char *path)
	{
	}

	void send(OscMessage & osc);
};
class LoArgs
{
public:
	LoArgs(const char *pchar, int index, lo_arg * a)
	{
		type = (lo_type) pchar[index];
		switch ((char) type)
		{
		case 's':
		{
			s = static_cast < char *>(&(a->s));
			break;
		}
		case 'i':
		{
			i = static_cast < int >(a->i);
			break;
		}

		}

	}
	lo_type type;
	int i;

	std::string s;
};

#endif
