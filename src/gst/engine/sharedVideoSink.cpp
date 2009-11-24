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

#include "./sharedVideoSink.h"
#include "./sharedVideoBuffer.h"
#include "util.h"

#include "./gstLinkable.h"
#include "./pipeline.h"
#include "videoSize.h"
#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>


using boost::interprocess::shared_memory_object;
using boost::interprocess::read_write;
using std::tr1::shared_ptr;


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
    catch (interprocess_exception &ex)
    {
        removeSharedMemory(id);
        LOG_ERROR("Got interprocess exception " << ex.what());
    }
    return shm;
}


bool SharedVideoSink::removeSharedMemory(const std::string &id)
{
    // Erase previously shared memory
    return shared_memory_object::remove(id.c_str());
}


SharedVideoSink::SharedVideoSink(const std::string &id) : 
    id_(id),
    colorspc_(0), 
    shm_(createSharedMemory(id_)), 
    region_(*shm_, read_write), // map the whole shared memory in this process
    sharedBuffer_(0)
{
    // get the address of the mapped region
    void *addr = region_.get_address();

    // construct the shared structure in memory with placement new
    sharedBuffer_ = new (addr) SharedVideoBuffer;

    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", NULL);
    sink_ = Pipeline::Instance()->makeElement("appsink", NULL);
    gstlinkable::link(colorspc_, sink_);
    prepareSink();
}


void SharedVideoSink::onNewBuffer(GstElement *elt, SharedVideoSink *context)
{
    using boost::interprocess::scoped_lock;
    using boost::interprocess::interprocess_mutex;
    using boost::interprocess::interprocess_exception;

    static unsigned long long bufferCount = 0;
    GstBuffer *buffer = 0;
    size_t size;

    try
    {
        /* get the buffer from appsink */
        buffer = gst_app_sink_pull_buffer(GST_APP_SINK(elt));

        // lock the mutex
        scoped_lock<interprocess_mutex> lock(context->sharedBuffer_->getMutex());

        // if a buffer has been pushed, wait until the consumer tells us
        // it's consumed it. note that upon waiting the mutex is released and will be
        // reacquired when this process is notified by the consumer.
        context->sharedBuffer_->waitOnConsumer(lock);

        if (context->sharedBuffer_->isPushing())
        {
            // push the buffer
            size = GST_BUFFER_SIZE (buffer);
            context->sharedBuffer_->pushBuffer(GST_BUFFER_DATA(buffer), size);
            ++bufferCount;
        }

        context->sharedBuffer_->notifyConsumer();
        // mutex is released here (goes out of scope)
    }
    catch (interprocess_exception &ex)
    {
        removeSharedMemory(context->id_);
        LOG_ERROR(ex.what());
        /* we don't need the appsink buffer anymore */
        gst_buffer_unref(buffer);
        return;
    }

    /* we don't need the appsink buffer anymore */
    gst_buffer_unref(buffer);
}


void SharedVideoSink::prepareSink()
{
    // FIXME: fixed caps are lame, should be bpp=12 to allow for 4 dc1394 cameras on one firewire port
    std::ostringstream capsStr;
    capsStr << "video/x-raw-rgb, width=" << videosize::WIDTH << ", height=" << videosize::HEIGHT
        << ", bpp=16, depth=16";
    GstCaps *videoCaps = gst_caps_from_string(capsStr.str().c_str());
    g_object_set(G_OBJECT(sink_), "emit-signals", TRUE, "caps", videoCaps, NULL);
    //g_object_set(sink_, "max-buffers", MAX_BUFFERS, "drop", TRUE, NULL);
    g_signal_connect(sink_, "new-buffer", G_CALLBACK(onNewBuffer), this);
    gst_caps_unref(videoCaps);
}


SharedVideoSink::~SharedVideoSink()
{
    destroySink();
    Pipeline::Instance()->remove(&colorspc_);
    removeSharedMemory(id_);
}

