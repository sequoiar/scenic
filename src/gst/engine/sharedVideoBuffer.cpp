#include "./sharedVideoBuffer.h"
#include "gutil.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/thread.hpp>

const double SharedVideoBuffer::ASPECT_RATIO = SharedVideoBuffer::WIDTH / SharedVideoBuffer::HEIGHT;

using namespace boost::interprocess;

SharedVideoBuffer::SharedVideoBuffer() : mutex_(), conditionEmpty_(), conditionFull_(), bufferIn_(false), hasSentinel_(false)
{}

interprocess_mutex & SharedVideoBuffer::getMutex()
{
    return mutex_;
}

unsigned char* SharedVideoBuffer::pixelsAddress()
{
    return pixels;
}

bool SharedVideoBuffer::hasSentinel() const
{
    return hasSentinel_;
}

void SharedVideoBuffer::pushBuffer(unsigned char *newBuffer, size_t size)
{
    // FIXME: dynamically sized buffer, changed by parameter size
    tassert(size == BUFFER_SIZE);
    memcpy(pixels, newBuffer, size);
}

void SharedVideoBuffer::pushSentinel()
{
    hasSentinel_ = true;
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
    while (bufferIn_)
    {
        conditionFull_.wait(lock);
    }
}


// wait for buffer to be pushed if it's currently empty
void SharedVideoBuffer::waitOnProducer(scoped_lock<interprocess_mutex> &lock)
{
    while (!bufferIn_)
    {
        conditionEmpty_.wait(lock);
    }
}

