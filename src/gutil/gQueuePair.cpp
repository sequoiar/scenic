/* QueuePair.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ARMapMsg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ARMapMsg is distributed in the hope that it will be useful,
 * but WITHOUMapMsg ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "queuePair.h"

void gQueuePair_::flip(gQueuePair_ &in)
{
    if(destroyQueues_)
        THROW_CRITICAL("QueuePair::flip called on QueuePair that owns queues i.e. init/flip mutually exclusive.");

    second_ = in.first_;
    first_ = in.second_;
}


MapMsg * gQueuePair_::timed_pop_(int ms)
{
    GTimeVal t;

    g_get_current_time(&t);
    g_time_val_add(&t, ms);
    return (static_cast <MapMsg* > (g_async_queue_timed_pop(first_, &t)));
}


MapMsg gQueuePair_::timed_pop(int microsec)
{
    MapMsg n;
    MapMsg *s = timed_pop_(microsec);

    if (s) {
        n = *s;
        delete s;
    }
    return n;
}


void gQueuePair_::push(MapMsg pt)
{
    MapMsg *t = new MapMsg(pt);

    push_(t);
}


gQueuePair_::~gQueuePair_()
{
    if (destroyQueues_)
    {
        MapMsg *t;
        do
        {
            t = static_cast<MapMsg*>(g_async_queue_try_pop(first_));
            delete t;
        }
        while(t);

        do
        {
            t = static_cast<MapMsg*>(g_async_queue_try_pop(second_));
            delete t;
        }
        while(t);

        g_async_queue_unref(first_);
        g_async_queue_unref(second_);
    }
}


/// Called in threadBase class for internal QueuePair not for a flipped copy 

void gQueuePair_::init()
{
    if (first_ != 0 || second_ != 0)
        THROW_CRITICAL("CALLED QueuePair::init() on non empty QueuePair. QueuePair::flip was probably called.");

    first_ = g_async_queue_new();
    second_ = g_async_queue_new();
    destroyQueues_ = true;
}


