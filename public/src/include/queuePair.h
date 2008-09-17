/* QueuePair_ 
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
 *		QueuePair_ asbstracts a pair of glib GAsyncQueues
 *      
 */
#ifndef __QUEUE_PAIR_H__
#define __QUEUE_PAIR_H__

#include <glib.h>
#include "logWriter.h"

class BaseQueuePair
{
    public:
        BaseQueuePair(GAsyncQueue* f, GAsyncQueue* s)
            : first_(f), second_(s){}
        virtual ~BaseQueuePair(){}

    protected:
        GAsyncQueue *first_, *second_;

    private:
        //No copying of class allowed
        BaseQueuePair(const BaseQueuePair& in);
        BaseQueuePair& operator=(const BaseQueuePair&);
};

/// QueuePair_ < T >
//  object of type T must be copyable
template < class T >
class QueuePair_
    : public BaseQueuePair
{
    public:
        QueuePair_ < T > ()
            : BaseQueuePair(0, 0), destroyQueues_(false) {}
        ~QueuePair_ < T > ();
        
        ///pop element or return T() if timeout
        T timed_pop(int microsec);
        void push(T pt);
        void flip(QueuePair_& in);
        void init();

    private:
        T* pop_() { return (static_cast < T* > (g_async_queue_pop(first_))); }
        void push_(T* t) { g_async_queue_push(second_, t); }

        T *timed_pop_(int ms);

        bool destroyQueues_;

        //No copying of class allowed 
        QueuePair_(const QueuePair_& in);
        QueuePair_& operator=(const QueuePair_&);
};


template < class T >
void QueuePair_< T >::flip(QueuePair_< T > &in)
{
    if(destroyQueues_)
        LOG_CRITICAL("QueuePair::flip called on QueuePair that owns queues i.e. init/flip mutually exclusive.");

    second_ = in.first_;
    first_ = in.second_;
}

template < class T >
T * QueuePair_ < T >::timed_pop_(int ms)
{
    GTimeVal t;

    g_get_current_time(&t);
    g_time_val_add(&t, ms);
    return (static_cast < T* > (g_async_queue_timed_pop(first_, &t)));
}

template < class T >
T QueuePair_ < T >::timed_pop(int microsec)
{
    T n;
    T *s = timed_pop_(microsec);

    if (s) {
        n = *s;
        delete s;
    }
    return n;
}

template < class T >
void QueuePair_ < T >::push(T pt)
{
    T *t = new T(pt);

    push_(t);
}

template < class T >
QueuePair_ < T >::~QueuePair_()
{
    if (destroyQueues_)
    {
        T *t;
        do
        {
            t = static_cast<T*>(g_async_queue_try_pop(first_));
            delete t;
        }
        while(t);

        do
        {
            t = static_cast<T*>(g_async_queue_try_pop(second_));
            delete t;
        }
        while(t);

        g_async_queue_unref(first_);
        g_async_queue_unref(second_);
    }
}


//Called in threadBase class for true QueuePair not for a flipped copy
template < class T >
void QueuePair_ < T >::init()
{
    if (first_ != 0 || second_ != 0)
        LOG_CRITICAL("CALLED QueuePair::init() on non empty QueuePair. QueuePair::flip was probably called.");

    first_ = g_async_queue_new();
    second_ = g_async_queue_new();
    destroyQueues_ = true;
}


#endif

