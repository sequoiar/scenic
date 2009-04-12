/* QueuePair.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __BOOST_QUEUE_PAIR_H__
#define __BOOST_QUEUE_PAIR_H__

#include "util.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition_variable.hpp>
#include <queue>
/** wraps pair of glib GAsyncQueue 
 * * provides cast void* to T* 
 * * note:object of type T must be copyable */
template < class T >
class QueuePair_
{
    public:
        QueuePair_ < T > ()
            :first_(0),second_(0), destroyQueues_(false),
        cond(),mut(),data_ready(){}
        ~QueuePair_ < T > ();
        
        ///pop element or return T() if timeout
        T timed_pop(int microsec);
        void push(T pt);
        void flip(QueuePair_& in);
        void init();

    private:
        std::queue<T*> *first_, *second_;
        T* pop_() { return queue_pop(); }
        void push_(T* t) { queue_push(t); }

        void queue_push(T *t);//{ second_->push(t);}
        T* queue_pop(boost::posix_time::ptime tm);
        T* queue_pop() { return queue_pop(0); }
        T *timed_pop_(int ms);

        bool destroyQueues_;
        boost::condition_variable cond;
        boost::mutex mut;
        bool data_ready;


        /** No Copy Constructor */
        QueuePair_(const QueuePair_& in);
        /** No Assignment Operator */
        QueuePair_& operator=(const QueuePair_&);
};

template < class T >
void QueuePair_< T >::queue_push(T *t)
{ 
    {
    boost::lock_guard<boost::mutex> lock(mut);
    second_->push(t);
    data_ready=true;
    }
    cond.notify_one();
}

template < class T >
T* QueuePair_< T >::queue_pop(boost::posix_time::ptime tm)
{
    T* ret;
    if(first_->empty())
    {
        if(!tm.is_not_a_date_time())
            while(tm < boost::posix_time::microsec_clock::local_time())
            {
                boost::unique_lock<boost::mutex> lock(mut);
                while(!data_ready)
                    cond.wait(lock);

                if(!first_->empty())
                    break;
            }
    }
    if(!first_->empty())
    {
        ret = first_->front(); 
        first_->pop();
    }
    return ret;
}

template < class T >
void QueuePair_< T >::flip(QueuePair_< T > &in)
{
    if(destroyQueues_)
        THROW_CRITICAL("QueuePair::flip called on QueuePair that owns queues i.e. init/flip mutually exclusive.");

    second_ = in.first_;
    first_ = in.second_;
}

template < class T >
T * QueuePair_ < T >::timed_pop_(int ms)
{
    using namespace boost::posix_time;
    ptime t = microsec_clock::local_time();

    ptime t2 = t + milliseconds(ms);
    return queue_pop(t2);// (static_cast < T* > (g_async_queue_timed_pop(first_, &t)));
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
        while(!first_->empty()) 
        {
            delete(first_->front());
            first_->pop();
        }

        while(!second_->empty()) 
        {
            delete(second_->front());
            second_->pop();
        }

        delete first_;
        delete second_;
    }
}


/// Called in threadBase class for internal QueuePair not for a flipped copy 
template < class T >
void QueuePair_ < T >::init()
{
    if (first_ != 0 || second_ != 0)
        THROW_CRITICAL("CALLED QueuePair::init() on non empty QueuePair. QueuePair::flip was probably called.");

    first_ = new std::queue<T*>();
    second_ = new std::queue<T*>();
    destroyQueues_ = true;
}


#endif

