/* GTHREAD-QUEUE-PAIR - Library of Thread Queue Routines for GLIB
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
#include "gThreadQueue.h"

namespace message {
    enum type 
    { 
        undefined, err, ok, ack, open, close, start, stop, pause, quit, info 
    };
    const char* str[] = 
    { 
        "undefined","err","ok","ack","open","close","start","stop","pause","quit","info" 
    };
};

struct Message
{
	Message(message::type m):type(m){}                    
    message::type type;
	void operator()()
    {  
        std::cout.flush(); 
        std::cout << "::" <<(long)g_thread_self()  << "-op()-" << message::str[type] << std::endl;
        std::cout.flush();
    }
};

void* thread_main(void* v)
{
    QueuePair &queue = *(static_cast<QueuePair*>(v));

	static Message r(message::ok);
    int count=0;
    while(1) 
    { 
        Message& f = *queue_pair_pop<Message*>(queue);
        f();
		queue_pair_push(queue,&r);
		if(count++ == 1000) 
		{
			static Message f(message::quit);			
			queue_pair_push(queue,&f);
	    return 0;
		}
    }
return 0; 
}


int main (int argc, char** argv) 
{ 
    GError *err=0;
    Message f(message::start);                                   
    
	
    g_thread_init(NULL);

    QueuePair queue (g_async_queue_new(), g_async_queue_new());
	InvertQueuePair tq(&queue);
    GThread* th = thread_create_queue_pair(thread_main,&tq,&err);
    
    while(err==NULL)
    {
		queue_pair_push(queue,&f);
		if(Message* f = queue_pair_timed_pop<Message*>(queue,10))
		{
			if(f->type == message::quit)
			  break;  
            (*f)();
		}
		
		std::cout << "sent it";
	}
	g_async_queue_unref(queue.first);
	g_async_queue_unref(queue.second);

	g_thread_join(th);
return 0;
}




