#include "./sharedVideoBuffer.h"
#include "gutil.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/thread.hpp>

const double SharedVideoBuffer::ASPECT_RATIO = SharedVideoBuffer::WIDTH / SharedVideoBuffer::HEIGHT;

using namespace boost::interprocess;

SharedVideoBuffer::SharedVideoBuffer() : mutex_(), conditionEmpty_(), conditionFull_(), bufferIn_(false), doPush_(true)
{}

interprocess_mutex & SharedVideoBuffer::getMutex()
{
    return mutex_;
}

unsigned char* SharedVideoBuffer::pixelsAddress()
{
    return pixels;
}

bool SharedVideoBuffer::isPushing() const
{
    return doPush_;
}

void SharedVideoBuffer::pushBuffer(unsigned char *newBuffer, size_t size)
{
    // FIXME: dynamically sized buffer, changed by parameter size
    if (size != BUFFER_SIZE)
    {
        LOG_ERROR("Cannot push unexpected video buffer size " << size << " to shared buffer\n");
        return;
    }
    memcpy(pixels, newBuffer, size);
}

void SharedVideoBuffer::stopPushing()
{
    doPush_ = false;
}


void SharedVideoBuffer::startPushing()
{
    doPush_ = true;
}

// notify the consumer process that there is a new buffer
void SharedVideoBuffer::notifyConsumer()
{
    conditionEmpty_.notify_one();
    // mark message buffer as full
    bufferIn_ = true;
}


void SharedVideoBuffer::notifyProducer()
{
    // Notify the other process that the buffer needs a refill 
    bufferIn_ = false;
    conditionFull_.notify_one();
}


void SharedVideoBuffer::waitOnConsumer(scoped_lock<interprocess_mutex> &lock)
{
    if (bufferIn_)
    {
        conditionFull_.wait(lock);
    }
}


// wait for buffer to be pushed if it's currently empty
void SharedVideoBuffer::waitOnProducer(scoped_lock<interprocess_mutex> &lock)
{
    if (!bufferIn_)
    {
        conditionEmpty_.wait(lock);
    }
}

