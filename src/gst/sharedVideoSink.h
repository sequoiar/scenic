// sharedVideoSink.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _SHARED_VIDEO_SINK_H_
#define _SHARED_VIDEO_SINK_H_

#include "./videoSink.h"
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/shared_ptr.hpp>

class _GtkWidget;
class _GstElement;
class SharedVideoBuffer;

class SharedVideoSink : public VideoSink 
{
    public:
        SharedVideoSink(const Pipeline &pipeline, int width, int height, const std::string& sharedId_);
        virtual ~SharedVideoSink();

    private:
         
        void prepareSink(int width, int height);
        static boost::shared_ptr<boost::interprocess::shared_memory_object> createSharedMemory(const std::string &id);
        static bool removeSharedMemory(const std::string &id);
        _GstElement *sinkElement() { return colorspc_; }

        const std::string id_;
        static const int MAX_BUFFERS = 1;
        _GstElement *colorspc_;
        boost::shared_ptr<boost::interprocess::shared_memory_object> shm_;
        boost::interprocess::mapped_region region_;
        SharedVideoBuffer *sharedBuffer_;

        static void onNewBuffer(_GstElement *elt, SharedVideoSink *context);
};

#endif  // _SHARED_VIDEO_SINK_H_
