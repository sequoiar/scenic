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

#ifndef __G_QUEUE_PAIR_H__
#define __G_QUEUE_PAIR_H__

#include "util.h"

#include <glib.h>
#include "mapMsg.h"
/** wraps pair of glib GAsyncQueue 
 * * provides cast void* to T* 
 * * note:object of type MapMsg must be copyable */

class gQueuePair_
{
    public:
        gQueuePair_()
            : first_(0), second_(0), destroyQueues_(false) {}
        ~gQueuePair_();
        
        ///pop element or return MapMsg() if timeout
        MapMsg timed_pop(int microsec);
        void push(MapMsg pt);
        void flip(gQueuePair_& in);
        void init();

    private:
        GAsyncQueue *first_, *second_;
        MapMsg* pop_() { return (static_cast <MapMsg* > (g_async_queue_pop(first_))); }
        void push_(MapMsg* t) { g_async_queue_push(second_, t); }

        MapMsg *timed_pop_(int ms);

        bool destroyQueues_;

        /** No Copy Constructor */
        gQueuePair_(const gQueuePair_& in);
        /** No Assignment Operator */
        gQueuePair_& operator=(const gQueuePair_&);
};

#endif

