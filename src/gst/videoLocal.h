
// videoLocal.h
// Copyright 2008 Koya Charles & Tristan Matthews
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

#ifndef _VIDEO_LOCAL_H_
#define _VIDEO_LOCAL_H_

#include "mediaBase.h"
#include "videoConfig.h"

class VideoSource;
class VideoSink;

class VideoLocal
    : public LocalBase
{
    public:
        /// Constructor 
        explicit VideoLocal(const VideoSourceConfig srcConfig, const VideoSinkConfig sinkConfig) 
            : srcConfig_(srcConfig), sinkConfig_(sinkConfig), source_(0), sink_(0) {}

        /// Destructor 
        ~VideoLocal();

        void start();

    private:
        void init_source();
        void init_sink();

        const VideoSourceConfig srcConfig_;
        const VideoSinkConfig sinkConfig_;
        VideoSource *source_;
        VideoSink *sink_;

        // hidden

        VideoLocal(const VideoLocal&); //No Copy Constructor
        VideoLocal& operator=(const VideoLocal&); //No Assignment Operator
};

#endif // _VIDEO_LOCAL_H_

