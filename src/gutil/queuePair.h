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
            : first_(f), second_(s){}
        virtual ~BaseQueuePair(){}

    protected:
        GAsyncQueue *first_, *second_;

    private:
        BaseQueuePair(const BaseQueuePair& in);
        BaseQueuePair& operator=(const BaseQueuePair&); //No Assignment Operator
};


template < class T >
class QueuePair_
    : public BaseQueuePair
{
    public:
        QueuePair_ < T > ()
            : BaseQueuePair(0, 0), destroyQueues_(false)
        {}

        ~QueuePair_ < T > ();

        T timed_pop(int ms);
        void push(T pt);
        void flip(QueuePair_& in);

        void init();

    protected:
        T* pop_();
        void push_(T* t);

    private:
        QueuePair_(const QueuePair_& in);
        QueuePair_& operator=(const QueuePair_&); //No Assignment Operator
        T *timed_pop_(int ms);

        bool destroyQueues_;

        static GMutex *mutex_;
};


template < class T >
T* QueuePair_< T >::pop_()
{
    return (static_cast < T* > (g_async_queue_pop(first_)));
}


template < class T >
void QueuePair_< T >::push_(T* t)
{
    g_async_queue_push(second_, t);
}


template < class T >
void QueuePair_< T >::flip(QueuePair_< T > &in)
{
    second_ = in.first_;
    first_ = in.second_;
}


template < class T >
GMutex * QueuePair_ < T >::mutex_ = NULL;


template < class T >
T * QueuePair_ < T >::timed_pop_(int ms)
{
    GTimeVal t;

    g_get_current_time(&t);
    g_time_val_add(&t, ms);
    return (static_cast < T* > (g_async_queue_timed_pop(first_, &t)));
}


template < class T >
T QueuePair_ < T >::timed_pop(int ms)
{
    T n;
    T *s = timed_pop_(ms);

    if (s) {
        n = *s;
        delete s;
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

    push_(t);
    g_mutex_unlock(mutex_);
}


template < class T >
QueuePair_ < T >::~QueuePair_()
{
    g_mutex_lock(mutex_);
    if (destroyQueues_)
    {
        T *t;
        do
        {
            t = static_cast<T*>(g_async_queue_try_pop(first_));
            if(t)
                delete t;
        }
        while(t);

        do
        {
            t = static_cast<T*>(g_async_queue_try_pop(second_));
            if(t)
                delete t;
        }
        while(t);

        g_async_queue_unref(first_);
        g_async_queue_unref(second_);
    }
    g_mutex_unlock(mutex_);
}


template < class T >
void QueuePair_ < T >::init()
{
    if (!mutex_)
        mutex_ = g_mutex_new();
    if (first_ == 0 && second_ == 0)
    {
        first_ = g_async_queue_new();
        second_ = g_async_queue_new();
        destroyQueues_ = true;
    }
}


#endif

