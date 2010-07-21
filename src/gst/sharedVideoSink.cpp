/* 
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 *
 * http://www.sat.qc.ca
 * All rights reserved.
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"
#include "sharedVideoSink.h"
#include "sharedVideoBuffer.h"

#include "gstLinkable.h"
#include "pipeline.h"
#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/thread_time.hpp>


using boost::interprocess::shared_memory_object;
using boost::interprocess::read_write;
using boost::shared_ptr;


shared_ptr<shared_memory_object> SharedVideoSink::createSharedMemory(const std::string &id)
{
    using boost::interprocess::create_only;
    using boost::interprocess::interprocess_exception;
    shared_ptr<shared_memory_object> shm;

    try 
    {
        removeSharedMemory(id);
        // create a shared memory object
        shm.reset(new shared_memory_object(create_only, id.c_str(), read_write)); 
        // set size
        shm->truncate(sizeof(SharedVideoBuffer));
    }
    catch (const interprocess_exception &ex)
    {
        removeSharedMemory(id);
        throw;
    }
    return shm;
}


bool SharedVideoSink::removeSharedMemory(const std::string &id)
{
    // Erase previously shared memory
    return shared_memory_object::remove(id.c_str());
}


SharedVideoSink::SharedVideoSink(const Pipeline &pipeline, 
        int width, int height, const std::string &id) 
:
    VideoSink(pipeline),
    id_(id),
    colorspc_(0), 
    shm_(createSharedMemory(id_)), 
    region_(*shm_, read_write), // map the whole shared memory in this process
    sharedBuffer_(0)
{
    // get the address of the mapped region
    void *addr = region_.get_address();

    // construct the shared structure in memory with placement new
    sharedBuffer_ = new (addr) SharedVideoBuffer(width, height);

    colorspc_ = pipeline_.makeElement("ffmpegcolorspace", NULL);
    sink_ = pipeline_.makeElement("appsink", NULL);
    gstlinkable::link(colorspc_, sink_);
    prepareSink(width, height);
}


void SharedVideoSink::onNewBuffer(GstElement *elt, SharedVideoSink *context)
{
    using boost::interprocess::scoped_lock;
    using boost::interprocess::interprocess_mutex;
    using boost::interprocess::interprocess_exception;

    GstBuffer *buffer = 0;
    size_t size;

    /// FIXME: maybe replace with Concurrent queue?
    try
    {
        /* get the buffer from appsink */
        buffer = gst_app_sink_pull_buffer(GST_APP_SINK(elt));

        // lock the mutex
        const boost::system_time timeout = boost::get_system_time() +
            boost::posix_time::seconds(5);
        scoped_lock<interprocess_mutex> lock(context->sharedBuffer_->getMutex(), timeout);
        if (not lock.owns())
            LOG_ERROR("Could not acquire shared memory mutex in 5 seconds or less.");

        // if a buffer has been pushed, wait until the consumer tells us
        // it's consumed it. note that upon waiting the mutex is released and will be
        // reacquired when this process is notified by the consumer.
        context->sharedBuffer_->waitOnConsumer(lock);

        // push the buffer
        size = GST_BUFFER_SIZE (buffer);
        context->sharedBuffer_->pushBuffer(GST_BUFFER_DATA(buffer), size);

        context->sharedBuffer_->notifyConsumer();
        // mutex is released here (goes out of scope)
    }
    catch (const interprocess_exception &ex)
    {
        LOG_WARNING("Got interprocess exception " << ex.what());
        removeSharedMemory(context->id_);
        /* we don't need the appsink buffer anymore */
        gst_buffer_unref(buffer);
        throw;
    }

    /* we don't need the appsink buffer anymore */
    gst_buffer_unref(buffer);
}


void SharedVideoSink::prepareSink(int width, int height)
{
    GstCaps *videoCaps; 

    std::ostringstream capsStr;

    /// FIXME: should detect caps from preceding element in pipeline if possible
    capsStr << "video/x-raw-rgb, width=" << width 
        << ", height=" << height << ",bpp=16, depth=16"; 
    videoCaps = gst_caps_from_string(capsStr.str().c_str());

    g_object_set(G_OBJECT(sink_), "emit-signals", TRUE, "caps", videoCaps, NULL);
    //g_object_set(sink_, "max-buffers", MAX_BUFFERS, "drop", TRUE, NULL);
    g_signal_connect(sink_, "new-buffer", G_CALLBACK(onNewBuffer), this);
    gst_caps_unref(videoCaps);
}


SharedVideoSink::~SharedVideoSink()
{
    // using boost::interprocess::interprocess_mutex;
    destroySink();
    pipeline_.remove(&colorspc_);
    // lock the mutex
    // boost::interprocess::scoped_lock<interprocess_mutex> lock(sharedBuffer_->getMutex());
    // sharedBuffer_->stopPushing();
    removeSharedMemory(id_);
}

