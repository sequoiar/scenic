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
#ifndef __BASE_THREAD_H__
#define __BASE_THREAD_H__

#include <glib.h>
#include <utility>
#include <list>
#include <map>
#include <string>
#include <set>
#include "baseModule.h"

typedef GAsyncQueue GAsyncQueue;

typedef std::pair < GAsyncQueue *, GAsyncQueue * >BaseQueuePair;

template < class T > class QueuePair_ : public BaseQueuePair
{
public:
    QueuePair_ < T > (GAsyncQueue * f, GAsyncQueue * s) : BaseQueuePair(f, s)
    {
    }
    QueuePair_ < T > () : BaseQueuePair(0, 0)
    {
    }
    ~QueuePair_ < T > ();
    T copy_timed_pop(int ms);
    void push(T pt);

    void done(T t);
    void done(T * pt);
    void init();
    typedef std::set < GAsyncQueue * >SetOfQueues;
    void del(bool);

private:
    static SetOfQueues qstor;
    bool own[2];
    T *timed_pop(int ms);
    static GMutex *mutex;
    static std::list < T * >l;

};

template < class T > std::set < GAsyncQueue * >QueuePair_ < T >::qstor;




template < class T > class BaseThread : public BaseModule
{
public:
    BaseThread < T > ();
    virtual ~BaseThread < T > ();

    QueuePair_ < T > getQueue(std::string s);
    GAsyncQueue *getPushQueue()
    {
        return queue.first;
    }
    GAsyncQueue *getPopQueue()
    {
        return queue.second;
    }
    bool run();
    virtual bool pre_run(){
        return true;
    }

protected:
    virtual int main()
    {
        return 0;
    }
    GThread *th;

    QueuePair_ < T > queue;
    std::map < std::string, QueuePair_ < T > >queue_map;
    static void *thread_main(void *v);
};

template < class T > QueuePair_ < T > BaseThread < T >::getQueue(std::string s)
{
    if (s.empty())
        return (QueuePair_ < T > (queue.second, queue.first));

    if (queue_map.find(s) == queue_map.end()) {

        QueuePair_ < T > qp = QueuePair_ < T > (queue.first, 0);
        qp.init();
        queue_map[s] = qp;
    }

    return QueuePair_ < T > (queue_map[s].second, queue_map[s].first);
}

template < class T > T* queue_pair_pop(BaseQueuePair qp)
{
    return (static_cast < T* > (g_async_queue_pop(qp.first)));
}

template < class T > void queue_pair_push(BaseQueuePair qp, T* t)
{
    g_async_queue_push(qp.second, t);
}

template < class T > T* queue_pair_timed_pop(BaseQueuePair* p, int ms)
{
    GTimeVal t;

    g_get_current_time(&t);
    g_time_val_add(&t, ms);
    return (static_cast < T* > (g_async_queue_timed_pop(p->first, &t)));
}

template < class T > GThread * thread_create_queue_pair(void *(thread) (void *), T t, GError ** err)
{
    return (g_thread_create(thread, static_cast < void *>(t), TRUE, err));
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

template < class T > GMutex * QueuePair_ < T >::mutex = NULL;

template < class T > std::list < T * >QueuePair_ < T >::l;





template < class T > T * QueuePair_ < T >::timed_pop(int ms)
{
    typename SetOfQueues::iterator it;
/*
    for (it = qstor.begin(); it != qstor.end(); ++it)
    {
        if (g_async_queue_length((*it)) > 0)
            if (*it == first)
                return (queue_pair_timed_pop < T >(this, ms));
    }
*/
                return (queue_pair_timed_pop < T >(this, ms));
    return 0;
}

/// copy time pop stuff.
//
template < class T > T QueuePair_ < T >::copy_timed_pop(int ms)
{
    T n;
    T *s = timed_pop(ms);

    if (s) {
        n = *s;
        done(s);
    }
    else
        n = T();

    return n;
}

template < class T > void QueuePair_ < T >::push(T pt)
{
    g_mutex_lock(mutex);
    T *t = new T(pt);

    l.push_front(t);

    queue_pair_push(*this, t);
    g_mutex_unlock(mutex);
}

template < class T > void QueuePair_ < T >::done(T * t)
{
    typename std::list<T*>::iterator it;

    g_mutex_lock(mutex);

    for(it = l.begin(); it != l.end(); ++it)
    {
        if(*it == t) {
            delete (t);
            l.remove(t);
            break;
        }
    }

    g_mutex_unlock(mutex);
}

template < class T > void QueuePair_ < T >::init()
{
    if (!mutex)
        mutex = g_mutex_new();

    if (first == 0) {
        own[0] = true;
        first = g_async_queue_new();
        qstor.insert(first);
    }
    else
        own[0] = false;
    if (second == 0) {
        own[1] = true;
        second = g_async_queue_new();
        qstor.insert(second);
    }
    else
        own[1] = false;

}
template < class T> void QueuePair_<T>::del(bool one)
{
    T *t;
    GAsyncQueue *q;
    if(one)
        q = first;
    else
        q = second;
    do
    {
        t = static_cast<T*>(g_async_queue_try_pop(q));
        if(t)
            delete t;
    }
    while(t);

    g_async_queue_unref(q);
    qstor.erase(q);
}

template < class T > QueuePair_<T>::~QueuePair_ ()
{

}

template < class T > BaseThread < T >::BaseThread() : th(0)
{
    if (!g_thread_supported ()) g_thread_init (NULL);
    queue.init();
    queue_map[""] = queue;
}

template < class T > BaseThread < T >::~BaseThread()
{
    if (th)
        g_thread_join(th);

    queue.del(true);
    //queue.del(false);
    for(; queue_map.begin() != queue_map.end(); queue_map.erase(queue_map.begin()))
        queue_map.begin()->second.del(false);
//	for (; queue_map.begin() != queue_map.end(); queue_map.erase(queue_map.begin()))
//            delete(*queue_map.begin());
}


template < class T > bool BaseThread < T >::run()
{
    GError *err = 0;

    //No thread yet
    if (th)
        return false;

    if(!pre_run())  //for derived classes
        return false;


    th = thread_create_queue_pair(BaseThread::thread_main, this, &err);

    if (th)  //BaseThread running
        return true;

    return false;
}

template < class T > void *BaseThread < T >::thread_main(void *v)
{
    return ((void *) (static_cast < BaseThread * >(v)->main()));
}

#endif
