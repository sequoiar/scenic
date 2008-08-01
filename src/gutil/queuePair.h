/* baseThread - Thread/AsyncQueue Routines using GLIB
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

/** \file
 *		Threads and mutexed message passing system
 *
 *
 */
#ifndef __QUEUE_PAIR_H__
#define __QUEUE_PAIR_H__

#include <glib.h>
#include <list>

typedef GAsyncQueue GAsyncQueue;


class BaseQueuePair
{
    public:
        BaseQueuePair(GAsyncQueue* f, GAsyncQueue* s)
            : first(f), second(s){}
        virtual ~BaseQueuePair(){}

        GAsyncQueue *first, *second;
        BaseQueuePair(const BaseQueuePair& in)
            : first(in.first), second(in.second){}

    private:
        BaseQueuePair& operator=(const BaseQueuePair&); //No Assignment Operator
};


template < class T >
class QueuePair_
    : public BaseQueuePair
{
    public:
        QueuePair_ < T > (GAsyncQueue * f, GAsyncQueue * s)
            : BaseQueuePair(f, s)
        {}
        QueuePair_ < T > ()
            : BaseQueuePair(0, 0)
        {}
        ~QueuePair_ < T > ();

        T timed_pop(int ms);
        void push(T pt);

        void done(T * pt);
        void init();

        void del(bool);

    private:
        bool own_[2];
        T *timed_pop_(int ms);

        static GMutex *mutex_;
        static std::list < T * >l_;
};


template < class T >
T* queue_pair_pop(BaseQueuePair qp)
{
    return (static_cast < T* > (g_async_queue_pop(qp.first)));
}


template < class T >
void queue_pair_push(BaseQueuePair qp, T* t)
{
    g_async_queue_push(qp.second, t);
}


template < class T >
T* queue_pair_timed_pop(BaseQueuePair* p, int ms)
{
    GTimeVal t;

    g_get_current_time(&t);
    g_time_val_add(&t, ms);
    return (static_cast < T* > (g_async_queue_timed_pop(p->first, &t)));
}



template < class T >
GMutex * QueuePair_ < T >::mutex_ = NULL;

template < class T >
std::list < T * >QueuePair_ < T >::l_;


template < class T >
T * QueuePair_ < T >::timed_pop_(int ms)
{
    return (queue_pair_timed_pop < T >(this, ms));
}


template < class T >
T QueuePair_ < T >::timed_pop(int ms)
{
    T n;
    T *s = timed_pop_(ms);

    if (s) {
        n = *s;
        done(s);
    }
    else
        n = T();
    return n;
}


template < class T >
void QueuePair_ < T >::push(T pt)
{
    g_mutex_lock(mutex_);
    T *t = new T(pt);

    l_.push_front(t);

    queue_pair_push(*this, t);
    g_mutex_unlock(mutex_);
}


template < class T >
void QueuePair_ < T >::done(T * t)
{
    typename std::list<T*>::iterator it;

    g_mutex_lock(mutex_);

    for(it = l_.begin(); it != l_.end(); ++it)
    {
        if(*it == t) {
            delete (t);
            l_.remove(t);
            break;
        }
    }

    g_mutex_unlock(mutex_);
}


template < class T >
QueuePair_ < T >::~QueuePair_()
{
    typename std::list<T*>::iterator it;

    g_mutex_lock(mutex_);

    for(it = l_.begin(); it != l_.end(); ++it)
    {
            delete (*it);
            l_.remove(*it);
    }

    g_mutex_unlock(mutex_);
}


template < class T >
void QueuePair_ < T >::init()
{
    if (!mutex_)
        mutex_ = g_mutex_new();
    if (first == 0) {
        own_[0] = true;
        first = g_async_queue_new();
    }
    else
        own_[0] = false;
    if (second == 0) {
        own_[1] = true;
        second = g_async_queue_new();
    }
    else
        own_[1] = false;
}


template < class T>
void QueuePair_<T>::del(bool one)
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
}


#endif

