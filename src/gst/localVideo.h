
// localVideo.h
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

#ifndef _LOCAL_VIDEO_H_
#define _LOCAL_VIDEO_H_

#include "noncopyable.h"
#include <tr1/memory>

class Pipeline;
class VideoSource;
class VideoSourceConfig;
class VideoScale;
class TextOverlay;
class VideoFlip;
class VideoSink;
class VideoSinkConfig;

class LocalVideo : private boost::noncopyable
{
    public:
        LocalVideo(Pipeline &pipeline, const VideoSourceConfig &sourceConfig,
                const VideoSinkConfig &sinkConfig);

    private:
        std::tr1::shared_ptr<VideoSource> source_;
        std::tr1::shared_ptr<VideoScale> videoscale_;
        std::tr1::shared_ptr<TextOverlay> textoverlay_;
        std::tr1::shared_ptr<VideoFlip> videoflip_;
        std::tr1::shared_ptr<VideoSink> sink_;
};

#endif // _LOCAL_VIDEO_H_

