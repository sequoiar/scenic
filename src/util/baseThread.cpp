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

#include "baseThread.h"

bool BaseThread::Quitted = false;


bool BaseThread::isQuitted(){
    return Quitted;
}


std::set< BaseThread *> BaseThread::allThreads_;


void BaseThread::postQuit(BaseThread* bt)
{
    QueuePair& qp = bt->queue_;
    LOG_DEBUG("postQuit");
    qp.push(MapMsg("quit"));
}


void BaseThread::broadcastQuit()
{
    for_each(allThreads_.begin(), allThreads_.end(), BaseThread::postQuit);
    Quitted = true;
}

/// client access to async QueuePair 

QueuePair &BaseThread::getQueue()
{
    return flippedQueue_;
}


BaseThread::BaseThread()
    : th_(0), queue_(), flippedQueue_()
{
    queue_.init();
    flippedQueue_.flip(queue_);
    allThreads_.insert(this);
}


BaseThread::~BaseThread()
{
    if (th_){
        MapMsg t("quit"); //TODO: this is forcing the 
        flippedQueue_.push(t);
        LOG_DEBUG("Thread Stopping " << this);
        allThreads_.erase(this);

        th_->join();
    }
}

class thread_create
{
    BaseThread* th_;
    public:
        thread_create(BaseThread* t):th_(t){}

        void operator()()
        {
            th_->main();
        }
};


/// entry point calls thread_create 

bool BaseThread::run()
{
    tassert(!th_);
    if (th_)
    {
        LOG_WARNING("Thread already Running.");
        return true;
    }
    //No thread yet
    if(!ready())
        return false;

    th_ = new boost::thread(thread_create(this));

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


