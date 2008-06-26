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

template <class T>
class QueuePair_ : public BaseQueuePair
{
public:
    QueuePair_<T>(GAsyncQueue*f,GAsyncQueue*s):BaseQueuePair(f,s){}
    QueuePair_<T>():BaseQueuePair(){}
    T* timed_pop(int ms){ return(queue_pair_timed_pop<T*>(*this,ms));}
    T copy_timed_pop(int ms){ T *s = timed_pop(ms); if(s) return *s; return T(); }
    void push(T pt){ queue_pair_push(*this,&pt);}
};
template <class T>
class BaseThread
{
public:
    BaseThread<T>();
    ~BaseThread<T>();

    QueuePair_<T> getInvertQueue(){return (QueuePair_<T>(queue.second,queue.first));}
    GAsyncQueue* getPushQueue(){return queue.first;}
    GAsyncQueue* getPopQueue(){return queue.second;}
    bool run();
protected:
    virtual int main(){}
    GThread* th;
    QueuePair_<T> queue;
static void* thread_main(void* v);
};


template <class T>
BaseThread<T>::BaseThread():th(0)
{
    g_thread_init(NULL);
    queue.first = g_async_queue_new();
    queue.second = g_async_queue_new();
}

template <class T>
BaseThread<T>::~BaseThread()
{
    if(th)
    	g_thread_join(th);

	g_async_queue_unref(queue.first);
	g_async_queue_unref(queue.second);

}

template <class T>
bool BaseThread<T>::run()
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


template <class T>
void* BaseThread<T>::thread_main(void* v)
{
    return((void*)(static_cast<BaseThread*>(v)->main()));
}
