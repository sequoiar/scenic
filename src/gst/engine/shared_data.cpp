#include "./shared_data.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/thread.hpp>

const double shared_data::ASPECT_RATIO = shared_data::WIDTH / shared_data::HEIGHT;

using namespace boost::interprocess;

shared_data::shared_data() : mutex_(), conditionEmpty_(), conditionFull_(), bufferIn_(false), hasSentinel_(false)
{}

interprocess_mutex & shared_data::getMutex()
{
    return mutex_;
}

unsigned char* shared_data::pixelsAddress()
{
    return pixels;
}

bool shared_data::hasSentinel() const
{
    return hasSentinel_;
}

void shared_data::pushBuffer(unsigned char *newBuffer, size_t size)
{
    // FIXME: dynamically sized buffer, changed by parameter size
    assert(size == BUFFER_SIZE);
    memcpy(pixels, newBuffer, size);
}

void shared_data::pushSentinel()
{
    hasSentinel_ = true;
}

// notify the consumer process that there is a new buffer
void shared_data::notifyConsumer()
{
    conditionEmpty_.notify_one();
    // mark message buffer as full
    bufferIn_ = true;
}


void shared_data::notifyProducer()
{
    // Notify the other process that the buffer needs a refill 
    bufferIn_ = false;
    conditionFull_.notify_one();
}


void shared_data::waitOnConsumer(scoped_lock<interprocess_mutex> &lock)
{
    while (bufferIn_)
    {
        conditionFull_.wait(lock);
    }
}


// wait for buffer to be pushed if it's currently empty
void shared_data::waitOnProducer(scoped_lock<interprocess_mutex> &lock)
{
    while (!bufferIn_)
    {
        conditionEmpty_.wait(lock);
    }
}

