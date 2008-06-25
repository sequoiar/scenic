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

typedef GAsyncQueue GAsyncQueue;

typedef std::pair<GAsyncQueue*,GAsyncQueue*> QueuePair;

class InvertQueuePair: public QueuePair
{
public:
	InvertQueuePair(QueuePair *q)                             
	:QueuePair(q->second,q->first)
	{}                         
	InvertQueuePair(QueuePair &q)                             
	:QueuePair(q.second,q.first)
	{}                         
	InvertQueuePair(GAsyncQueue*f,GAsyncQueue*s)                             
	:QueuePair(s,f)
	{}                         
};
 
template <class T>
T queue_pair_pop(QueuePair qp)
{                                                
	return(static_cast<T>(g_async_queue_pop(qp.first)));
}

template <class T>
void queue_pair_push(QueuePair qp,T t)
{
	g_async_queue_push(qp.second,t);
}
                                                     
template<class T>
T queue_pair_timed_pop(QueuePair p,int ms)
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

class BaseThread
{
public:
    BaseThread();
    ~BaseThread();

    QueuePair getInvertQueue(){return InvertQueuePair(queue);}
    bool run();
private:
    virtual int main(){}
    GThread* th;
    QueuePair queue;
static void* thread_main(void* v);
};



