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
#ifdef HAVE_BOOST_THREAD
#include <boost/thread.hpp>
#else
#include <glib.h>
#endif

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

        static void broadcastQuit();
        static bool isQuitted();
        virtual void main() { }
    protected:
        static void *thread_main(void *pThreadObj);
        void operator()(){main();}
        virtual bool ready() { return true; }
        static void postQuit(BaseThread<T>* bt);
#ifdef HAVE_BOOST_THREAD
        boost::thread *th_;
#else
        GThread *th_;
#endif
        QueuePair_ < T > queue_;
        QueuePair_ < T > flippedQueue_;

        static std::set< BaseThread < T > *> allThreads_;
        static bool Quitted;
    private:
        /** No Copy Constructor */
        BaseThread(const BaseThread&); 
        /** No Assignment Operator */
        BaseThread& operator=(const BaseThread&); 
};

template < class T >
bool BaseThread< T >::Quitted = false;

template < class T >
bool BaseThread< T >::isQuitted(){
    return Quitted;
}

template < class T >
std::set< BaseThread < T > *> BaseThread<T>::allThreads_;

template < class T >
void BaseThread < T >::postQuit(BaseThread<T>* bt)
{
    QueuePair_ <T> & qp = bt->queue_;
    LOG_DEBUG("postQuit");
    qp.push(T("quit"));
}

template < class T >
void BaseThread < T >::broadcastQuit()
{
    for_each(allThreads_.begin(), allThreads_.end(), BaseThread<T>::postQuit);
    Quitted = true;
}

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
#ifndef HAVE_BOOST_THREAD
    if (!g_thread_supported ())
        g_thread_init (NULL);
#endif
    queue_.init();
    flippedQueue_.flip(queue_);
    allThreads_.insert(this);
}

template < class T >
BaseThread < T >::~BaseThread()
{
    if (th_){
        T t("quit"); //TODO: this is forcing the template param to have char* constructor
        flippedQueue_.push(t);
        LOG_DEBUG("Thread Stoping " << this);
        allThreads_.erase(this);
#ifdef HAVE_BOOST_THREAD
        th_->join();
#else
        g_thread_join(th_);
#endif
    }
}

#ifdef HAVE_BOOST_THREAD
template < class T >
class thread_create
{
    BaseThread<T>* th_;
    public:
        thread_create(BaseThread<T>* t):th_(t){}

        void operator()()
        {
            th_->main();
        }
};
#else
/// thread entry point 

template < class T >
GThread * thread_create(void *(thread)(void *),T t, GError **err)
{
    return (g_thread_create(thread, static_cast < void *>(t), TRUE, err)); 
}

template < class T >
void *BaseThread < T >::thread_main(void *pThreadObj)
{
    static_cast < BaseThread * >(pThreadObj)->main();
    return 0;
}
#endif

/// entry point calls thread_create 
template < class T >
bool BaseThread < T >::run()
{
    if (th_)
    {
        LOG_WARNING("Thread already Running.");
        return true;
    }
    //No thread yet
    if(!ready())
        return false;

#ifdef HAVE_BOOST_THREAD
    th_ = new boost::thread(thread_create<T>(this));
#else
    GError *err = 0;
    th_ = thread_create(BaseThread::thread_main, this, &err);
#endif

    LOG_DEBUG("Thread Started " << this);

    if (!th_)       //BaseThread failed
    {
        THROW_CRITICAL("thread_create failed!");
        return false;
    }
    usleep(1);      // Insure thread started or g_thread_join 
                    //returns before this thread starts
    return true;
}

#endif

