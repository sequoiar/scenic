
/* videoFlip.h
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

#ifndef _VIDEO_FLIP_H_
#define _VIDEO_FLIP_H_

#include "noncopyable.h"

// forward declarations
class Pipeline;
class _GstElement;

/**
 *  A filter that scales video to a specified resolution.
 */

class VideoFlip : private boost::noncopyable
{
    public:
        VideoFlip(const Pipeline &pipeline, const std::string &flipMethod);
        _GstElement *sinkElement();
        _GstElement *srcElement();


    private:
        _GstElement *identity_;
        _GstElement *colorspace_;
        _GstElement *videoflip_;
};

#endif //_VIDEO_FLIP_H_

