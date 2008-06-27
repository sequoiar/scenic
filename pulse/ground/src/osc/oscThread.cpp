
// python_import.cpp
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

/** \filep
 *      This file gets included in python module
 *
 *
 *      Exposes object modules to python interpreter.
 *
 */

#include <iostream>


#include "thread/baseThread.h"
//#include "thread/message.h"
#include <lo/lo.h>
#include "oscThread.h"


int OscThread::generic_handler_static(const char *path, const char *types, lo_arg **argv, int argc,
                void *data, void *user_data)
{
   OscThread* t = static_cast<OscThread*>(user_data);
   return (t->generic_handler(path,types,argv,argc,data));
}


int OscThread::generic_handler(const char *path, const char *types, lo_arg **argv, int argc,
                void *data)
{
   queue.push(OscMessage(path,types,argv,argc,data));
}




int OscThread::main()
{
//	static Message r(message::ok);
    int count=0;

    lo_server_thread st = lo_server_thread_new("7770", liblo_error);

    lo_server_thread_add_method(st, NULL,NULL, generic_handler_static, this);

    lo_server_thread_start(st);

    while(1) 
    { 
        OscMessage msg = queue.copy_timed_pop(1000);
        if (!msg.path.empty())
        {
            queue.push(msg);
        }


    }
return 0; 
}




/*		queue_pair_push(queue,&r);
		if(count++ == 1000) 
		{
			static Message f(message::quit);			
			queue_pair_push(queue,&f);
    	    break;
		}
*/

