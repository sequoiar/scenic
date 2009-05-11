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
#ifdef HAVE_BOOST_THREAD

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition_variable.hpp>
#include <queue>
#include "mapMsg.h"
/** wraps pair of queues 
 * note:object of type T must be copyable
 * */

class QueuePair_
{
    public:
        QueuePair_ ()
            :first_(0),second_(0), destroyQueues_(false),
        cond(),mut(),data_ready(){}
        ~QueuePair_ ();
        
        ///pop element or return T() if timeout
        MapMsg timed_pop(int microsec);
        void push(MapMsg pt);
        void flip(QueuePair_& in);
        void init();
        bool ready();

    private:
        std::queue<MapMsg> *first_, *second_;

//        void queue_push(MapMsg t);
//        MapMsg queue_pop(int ms);

        bool destroyQueues_;
        boost::condition_variable cond;
        boost::mutex mut;
        volatile bool data_ready;


        /** No Copy Constructor */
        QueuePair_(const QueuePair_& in);
        /** No Assignment Operator */
        QueuePair_& operator=(const QueuePair_&);
};

#endif


#endif

