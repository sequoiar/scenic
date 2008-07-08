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
#include "baseThread.h"
#include "baseMessage.h"

typedef QueuePair_<BaseMessage> QueuePair;

class Thread : public BaseThread<BaseMessage>
{
    int main();
};

int Thread::main()
{
	BaseMessage r(BaseMessage::ping);
    int count=0;
    while(1) 
    { 
        BaseMessage f = queue.copy_timed_pop(1);
		queue.push(r);
		if(count++ == 10000) 
		{
			BaseMessage f(BaseMessage::quit);			
			queue.push(f);
    	    break;
		}
    }
return 0; 
}


int main (int argc, char** argv) 
{ 
    Thread t;
	

    QueuePair queue = t.getQueue("");
    QueuePair queue2 = t.getQueue("a");
    if(!t.run())
        return -1;
    
    while(1)
    {
    	BaseMessage f(BaseMessage::ok);                                   
		queue.push(f);
		f = queue.copy_timed_pop(1);
		BaseMessage f2 = queue2.copy_timed_pop(1);
		
		if(f.get_type() == BaseMessage::quit){
			break;
		}

		
		
	}

    std::cout << "Done!" << std::endl;

return 0;
}




