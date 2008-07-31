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
#ifndef __BASE_THREAD_H__
#define __BASE_THREAD_H__

#include <glib.h>
#include <utility>
#include <list>
#include <map>
#include <string>
#include <set>
#include "baseModule.h"
#include "queuePair.h"


template < class T >
class BaseThread
    : public BaseModule
{
    public:
        BaseThread < T > ();
        virtual ~BaseThread < T > ();

        QueuePair_ < T > getQueue();

        GAsyncQueue *getPushQueue() {
            return queue.first;
        }


        GAsyncQueue *getPopQueue() {
            return queue.second;
        }


        bool run();
    protected:
        virtual int main() {
            return 0;
        }
        virtual bool ready(){ return true;}


        GThread *th;

        QueuePair_ < T > queue;
        static void *thread_main(void *v);

    private:
        BaseThread(const BaseThread&); //No Copy Constructor
        BaseThread& operator=(const BaseThread&); //No Assignment Operator
};

template < class T >
QueuePair_ < T > BaseThread < T >::getQueue()
{
    return (QueuePair_ < T > (queue.second, queue.first));
}



template < class T >
BaseThread < T >::BaseThread()
    : th(0), queue()
{
    if (!g_thread_supported ())
        g_thread_init (NULL);
    queue.init();
}


template < class T >
BaseThread < T >::~BaseThread()
{
    if (th)
        g_thread_join(th);
    queue.del(true);
    queue.del(false);
}


template < class T >
bool BaseThread < T >::run()
{
    GError *err = 0;

    //No thread yet
    if (th || !ready())
        return false;
    th = thread_create_queue_pair(BaseThread::thread_main, this, &err);

    if (th)  //BaseThread running
        return true;
    return false;
}


template < class T >
void *BaseThread < T >::thread_main(void *v)
{
    return (reinterpret_cast<void *>((static_cast < BaseThread * >(v)->main())));
}


#endif

