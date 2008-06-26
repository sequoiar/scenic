#include "baseThread.h"

BaseThread::BaseThread():th(0)
{
    g_thread_init(NULL);
    queue.first = g_async_queue_new();
    queue.second = g_async_queue_new();
}

BaseThread::~BaseThread()
{
    if(th)
    	g_thread_join(th);

	g_async_queue_unref(queue.first);
	g_async_queue_unref(queue.second);

}

bool BaseThread::run()
{
    GError *err=0;

    //No thread yet
    if(th)
        return false;

    th = thread_create_queue_pair(BaseThread::thread_main,this,&err);

    if(th) //BaseThread running
        return true;

    return false;
}


void* BaseThread::thread_main(void* v)
{
    return((void*)(static_cast<BaseThread*>(v)->main()));
}


