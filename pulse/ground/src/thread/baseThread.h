/* GTHREAD-QUEUE-PAIR - Library of BaseThread Queue Routines for GLIB
 * Copyright (C) 2008	Koya Charles, Tristan Matthews 
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
#include <utility>
#include "message.h"

typedef GAsyncQueue GAsyncQueue;

typedef std::pair<GAsyncQueue*,GAsyncQueue*> BaseQueuePair;


class InvertBaseQueuePair: public BaseQueuePair
{
public:
	InvertBaseQueuePair(BaseQueuePair *q)                             
	:BaseQueuePair(q->second,q->first)
	{}                         
	InvertBaseQueuePair(BaseQueuePair &q)                             
	:BaseQueuePair(q.second,q.first)
	{}                         
	InvertBaseQueuePair(GAsyncQueue*f,GAsyncQueue*s)                             
	:BaseQueuePair(s,f)
	{}                         
};
 
template <class T>
T queue_pair_pop(BaseQueuePair qp)
{                                                
	return(static_cast<T>(g_async_queue_pop(qp.first)));
}

template <class T>
void queue_pair_push(BaseQueuePair qp,T t)
{
	g_async_queue_push(qp.second,t);
}
                                                     
template<class T>
T queue_pair_timed_pop(BaseQueuePair p,int ms)
{
    GTimeVal t;
    g_get_current_time(&t);
    g_time_val_add(&t,ms);
    return(static_cast<T>(g_async_queue_timed_pop(p.first,&t)));
}

template<class T>
GThread* thread_create_queue_pair(void* (thread)(void*),T t,GError **err){
	return(g_thread_create(thread,static_cast<void*>(t),TRUE,err));
}
/*
class BaseQueuePair
{
    BaseQueuePair()
    GAsyncQueue* get_first(){ return queue
public:
    BaseQueuePair_ queue_

};
*/
class QueuePair : public BaseQueuePair
{
public:
    QueuePair(GAsyncQueue*f,GAsyncQueue*s):BaseQueuePair(f,s){}
    QueuePair():BaseQueuePair(){}
    Message* timed_pop(int ms){ return(queue_pair_timed_pop<Message*>(*this,ms));}
    Message copy_timed_pop(int ms){ Message *s = timed_pop(ms); if(s) return *s; return Message(message::undefined); }
    void push(Message pt){ queue_pair_push(*this,&pt);}
};

class BaseThread
{
public:
    BaseThread();
    ~BaseThread();

    QueuePair getInvertQueue(){return (QueuePair(queue.second,queue.first));}
    GAsyncQueue* getPushQueue(){return queue.first;}
    GAsyncQueue* getPopQueue(){return queue.second;}
    bool run();
protected:
    virtual int main(){}
    GThread* th;
    QueuePair queue;
static void* thread_main(void* v);
};


BaseThread::BaseThread():th(0)
{
    g_thread_init(NULL);
    queue.first = g_async_queue_new();
    queue.second = g_async_queue_new();
}

BaseThread::~BaseThread()
{
    if(th)
    	g_thread_join(th);

	g_async_queue_unref(queue.first);
	g_async_queue_unref(queue.second);

}

bool BaseThread::run()
{
    GError *err=0;

    //No thread yet
    if(th)
        return false;

    th = thread_create_queue_pair(BaseThread::thread_main,this,&err);

    if(th) //BaseThread running
        return true;

    return false;
}


void* BaseThread::thread_main(void* v)
{
    return((void*)(static_cast<BaseThread*>(v)->main()));
}
