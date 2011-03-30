
/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "shared_video_buffer.h"

#ifndef LOG_ERROR
#include <iostream> // for cerr
#endif

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/thread/thread_time.hpp>

/// FIXME: doesn't get updated
double SharedVideoBuffer::ASPECT_RATIO = videosize::WIDTH / videosize::HEIGHT;

using namespace boost::interprocess;

SharedVideoBuffer::SharedVideoBuffer(int width, int height) : width_(width), height_(height),
    mutex_(), conditionEmpty_(), conditionFull_(), bufferIn_(false)
{
    ASPECT_RATIO = width_ / height_;
}

SharedVideoBuffer::~SharedVideoBuffer()
{
}

interprocess_mutex & SharedVideoBuffer::getMutex()
{
    return mutex_;
}

unsigned char* SharedVideoBuffer::pixelsAddress()
{
    return pixels;
}

void SharedVideoBuffer::pushBuffer(unsigned char *newBuffer, size_t size)
{
    // FIXME: dynamically sized buffer, changed by parameter size
    if (size > MAX_BUFFER_SIZE)
    {
        /// this is so other applications can include SharedVideoBuffer without using our logging
#ifdef LOG_ERROR
        LOG_ERROR("Cannot push unexpected video buffer size " << size << " to shared buffer\n");
#else
        std::cerr << "Cannot push unexpected video buffer size " << size << " to shared buffer\n";
#endif
        return;
    }
    memcpy(pixels, newBuffer, size);
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


bool SharedVideoBuffer::waitOnConsumer(scoped_lock<interprocess_mutex> &lock)
{
    const boost::system_time timeout = boost::get_system_time() +
        boost::posix_time::milliseconds(10);

    if (bufferIn_)   // XXX: this must be an if, not a while, otherwise process hangs
    {
        return conditionFull_.timed_wait(lock, timeout);
    }
    return true;
}


// wait for buffer to be pushed if it's currently empty
bool SharedVideoBuffer::waitOnProducer(scoped_lock<interprocess_mutex> &lock)
{
    const boost::system_time timeout = boost::get_system_time() +
        boost::posix_time::seconds(5);

    if (!bufferIn_)  // XXX: this must be an if, not a while, otherwise process hangs
    {
        return conditionEmpty_.timed_wait(lock, timeout);
    }
    return true;
}


int SharedVideoBuffer::getWidth()
{
    return width_;
}


int SharedVideoBuffer::getHeight()
{
    return height_;
}

// Used by AC_CHECK_LIB in configure.ac
void shared_video_is_present()
{
}

