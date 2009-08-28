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

class _GtkWidget;
class _GstElement;
class shared_data;

class SharedVideoSink : public VideoSink 
{
    public:
        SharedVideoSink();
        virtual ~SharedVideoSink();
        virtual void init();

    private:
         
        virtual void handleMessage(const std::string &message);
        void prepareSink();
        _GstElement *sinkElement() { return colorspc_; }

        _GstElement *colorspc_;
        boost::interprocess::shared_memory_object *shm_;
        boost::interprocess::mapped_region *region_;
        shared_data *sharedBuffer_;

        static const std::string id_;

        static void onNewBuffer(_GstElement *elt, SharedVideoSink *context);
        SharedVideoSink(const SharedVideoSink&);
        SharedVideoSink& operator=(const SharedVideoSink&);
};

#endif  // _SHARED_VIDEO_SINK_H_
