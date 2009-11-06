
/* sharedVideoBuffer.cpp
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
 * [propulse]ART is distributed in the hope that it will be useful, * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"
#include "./sharedVideoBuffer.h"
#include "gutil.h"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/thread.hpp>

const double SharedVideoBuffer::ASPECT_RATIO = videosize::WIDTH / videosize::HEIGHT;

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
    if (size >= MAX_BUFFER_SIZE)
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
    boost::system_time const timeout = boost::get_system_time() +
        boost::posix_time::milliseconds(1);

    if (bufferIn_)   // XXX: this must be an if, not a while, otherwise process hangs 
    {
        conditionFull_.timed_wait(lock, timeout);
    }
}


// wait for buffer to be pushed if it's currently empty
void SharedVideoBuffer::waitOnProducer(scoped_lock<interprocess_mutex> &lock)
{
    if (!bufferIn_)  // XXX: this must be an if, not a while, otherwise process hangs
    {
        conditionEmpty_.wait(lock);
    }
}

