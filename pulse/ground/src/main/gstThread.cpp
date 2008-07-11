/* GTHREAD-QUEUE-PAIR - Library of GstThread Queue Routines for GLIB
 * Copyright 2008  Koya Charles & Tristan Matthews
 *
 * This library is free software; you can redistribute it and/or
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

#include <glib.h>
#include <iostream>
#include "gutil/baseThread.h"
#include "gutil/baseMessage.h"
#include "gutil/optionArgs.h"
#include "logWriter.h"

#include "gst/videoSender.h"
#include "gst/videoReceiver.h"
#include "gst/videoConfig.h"


typedef QueuePair_<BaseMessage> QueuePair;
class GstThread : public BaseThread<BaseMessage>
{
	VideoConfig* conf;
	VideoSender* sender;
	VideoReceiver* receiver;

	char* conf_str;
	int conf_i;
	int main();
	bool pre_run();
public:
	ArgList args();
};


BaseModule::ArgList GstThread::args()
{
	static ArgList args;
	args.clear();
	args.push_back(new StringArg(&conf_str,"vsrc",'s',"video", "try videotestsrc"));
	args.push_back(new IntArg(&conf_i,"nt",'j',"iiiiasdasd", "iiiitry videotestsrc"));
	args.push_back(new IntArg(&conf_i,"int",'i',"iiiiasdasd", "iiiitry videotestsrc"));
	return args;
}

bool GstThread::pre_run()
{
	return true;
}

int GstThread::main()
{
	BaseMessage r(BaseMessage::ping);
	int count=0;
	conf = new VideoConfig(conf_str);
	sender = new VideoSender(*conf);
	sender->init();
	sender->start();
	while(1)
	{
		BaseMessage f = queue.copy_timed_pop(1000);
		//queue.push(r);
		LOG(" here ");
		
		if(count++ == 1000) {
			BaseMessage f(BaseMessage::quit);
			queue.push(f);
			break;
		}
	}
	return 0;
}


int main (int argc, char** argv)
{
	GstThread t;
	OptionArgs opts;

	opts.add(t.args());
	if(!opts.parse(argc,argv))
		return 1;

	QueuePair queue = t.getQueue("");
//`	QueuePair queue2 = t.getQueue("a");
	if(!t.run())
		return -1;

	while(1)
	{
		BaseMessage f(BaseMessage::ok);
		queue.push(f);
		f = queue.copy_timed_pop(1);
		//BaseMessage f2 = queue2.copy_timed_pop(1);

		if(f.get_type() == BaseMessage::quit) {
			break;
		}



	}

	std::cout << "Done!" << std::endl;

	return 0;
}




