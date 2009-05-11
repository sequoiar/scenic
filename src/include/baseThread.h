/* baseThread.h
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
 *
 */

#ifndef __BASE_THREAD_H__
#define __BASE_THREAD_H__

#include <string>
#include <set>
#include <algorithm>

#include "config.h"
#include "baseModule.h"
#include "queuePair.h"
#ifdef HAVE_BOOST_THREAD
#include <boost/thread.hpp>
typedef QueuePair_ QueuePair;
#else
#include <glib.h>
typedef gQueuePair_ QueuePair;
#endif

/// glib thread construct with async queues 

class BaseThread
    : public BaseModule
{
    public:
        BaseThread ();
        virtual ~BaseThread ();

        QueuePair &getQueue();
        bool run();

        static void broadcastQuit();
        static bool isQuitted();
        virtual void main() { }
    protected:
        static void *thread_main(void *pThreadObj);
        void operator()(){main();}
        virtual bool ready() { return true; }
        static void postQuit(BaseThread* bt);
#ifdef HAVE_BOOST_THREAD
        boost::thread *th_;
#else
        GThread *th_;
#endif
        QueuePair queue_;
        QueuePair flippedQueue_;

        static std::set< BaseThread *> allThreads_;
        static bool Quitted;
    private:
        /** No Copy Constructor */
        BaseThread(const BaseThread&); 
        /** No Assignment Operator */
        BaseThread& operator=(const BaseThread&); 
};

#endif

