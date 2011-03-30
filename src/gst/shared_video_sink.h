//
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _SHARED_VIDEO_SINK_H_
#define _SHARED_VIDEO_SINK_H_

#include "video_sink.h"
#include <boost/interprocess/mapped_region.hpp>
#include <tr1/memory>

class _GtkWidget;
class _GstElement;
class SharedVideoBuffer;

namespace boost {
    namespace interprocess {
        class shared_memory_object;
    }
}
class SharedVideoSink : public VideoSink
{
    public:
        SharedVideoSink(const Pipeline &pipeline, int width, int height, const std::string& sharedId_);
        virtual ~SharedVideoSink();

    private:

        void prepareSink(int width, int height);
        static std::tr1::shared_ptr<boost::interprocess::shared_memory_object> createSharedMemory(const std::string &id);
        static bool removeSharedMemory(const std::string &id);
        virtual _GstElement *sinkElement() { return colorspc_; }

        const std::string id_;
        static const int MAX_BUFFERS = 1;
        _GstElement *colorspc_;
        std::tr1::shared_ptr<boost::interprocess::shared_memory_object> shm_;
        boost::interprocess::mapped_region region_;
        SharedVideoBuffer *sharedBuffer_;

        static void onNewBuffer(_GstElement *elt, SharedVideoSink *context);
};

#endif  // _SHARED_VIDEO_SINK_H_
