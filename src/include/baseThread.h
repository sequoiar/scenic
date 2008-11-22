/* baseThread.h
 * Copyright (C) 2008 Koya Charles, Tristan Matthews
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

#include <glib.h>
#include <string>
#include "baseModule.h"
#include "queuePair.h"

/// glib thread construct with async queues 
template < class T >
class BaseThread
    : public BaseModule
{
    public:
        BaseThread < T > ();
        virtual ~BaseThread < T > ();

        QueuePair_ < T > &getQueue();
        bool run();

    protected:
        virtual int main() { return 0; }
        static void *thread_main(void *pThreadObj);
        virtual bool ready() { return true; }

        GThread *th_;
        QueuePair_ < T > queue_;
        QueuePair_ < T > flippedQueue_;

    private:
        BaseThread(const BaseThread&); //No Copy Constructor
        BaseThread& operator=(const BaseThread&); //No Assignment Operator
};

/// client access to async QueuePair 
template < class T >
QueuePair_ < T > &BaseThread < T >::getQueue()
{
    return flippedQueue_;
}

template < class T >
BaseThread < T >::BaseThread()
    : th_(0), queue_(), flippedQueue_()
{
    if (!g_thread_supported ())
        g_thread_init (NULL);
    queue_.init();
    flippedQueue_.flip(queue_);
}

template < class T >
BaseThread < T >::~BaseThread()
{
    if (th_){
        T t("quit");
        flippedQueue_.push(t);
        g_thread_join(th_);
    }
}


/// thread creation 
template < class T >
GThread * thread_create(void *(thread) (void *), T t, GError ** err)
{
    return (g_thread_create(thread, static_cast < void *>(t), TRUE, err));
}


/// entry point calls thread_create 
template < class T >
bool BaseThread < T >::run()
{
    GError *err = 0;
    if (th_)
    {
        LOG_WARNING("Thread already Running.");
        return true;
    }
    //No thread yet
    if(!ready())
        return false;

    th_ = thread_create(BaseThread::thread_main, this, &err);

    if (!th_)       //BaseThread failed
    {
        THROW_CRITICAL("thread_create failed!");
        return false;
    }
    usleep(1);      // Insure thread started or g_thread_join 
                    //returns before this thread starts
    return true;
}

/// thread entry point 
template < class T >
void *BaseThread < T >::thread_main(void *pThreadObj)
{
    return reinterpret_cast<void *>(
            static_cast < BaseThread * >(pThreadObj)->main()
            );
}


#endif

