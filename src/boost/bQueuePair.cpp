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
#include "queuePair.h"

#ifdef HAVE_BOOST_THREAD
void QueuePair_::queue_push(MapMsg *t)
{ 
    {
    boost::lock_guard<boost::mutex> lock(mut);
    second_->push(t);
    data_ready=true;
    }
    cond.notify_one();
}


MapMsg* QueuePair_::queue_pop(int micro_secs)
{
    MapMsg* ret;
    boost::xtime xt;
    xtime_get(&xt,boost::TIME_UTC);
    xt.nsec += micro_secs*1000;

    boost::unique_lock<boost::mutex> lock(mut);
    while(!data_ready)
        if(!cond.timed_wait(lock,xt))
            break;

    if(!first_->empty())
    {
        ret = first_->front(); 
        first_->pop();
        data_ready=false;
        return ret;
    }
    return 0;
}


void QueuePair_::flip(QueuePair_ &in)
{
    if(destroyQueues_)
        THROW_CRITICAL("QueuePair::flip called on QueuePair that owns queues i.e. init/flip mutually exclusive.");

    second_ = in.first_;
    first_ = in.second_;
}



MapMsg QueuePair_::timed_pop(int microsec)
{
    MapMsg n;
    MapMsg *s = queue_pop(microsec);

    if (s) {
        LOG_DEBUG("");
        n = *s;
        delete s;
    }
    return n;
}


void QueuePair_::push(MapMsg pt)
{
    MapMsg *t = new MapMsg(pt);

    push_(t);
}


QueuePair_::~QueuePair_()
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

void QueuePair_::init()
{
    if (first_ != 0 || second_ != 0)
        THROW_CRITICAL("CALLED QueuePair::init() on non empty QueuePair. QueuePair::flip was probably called.");

    first_ = new std::queue<MapMsg*>();
    second_ = new std::queue<MapMsg*>();
    destroyQueues_ = true;
}

#endif
