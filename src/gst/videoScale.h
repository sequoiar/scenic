
/* videoScale.h
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
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _VIDEO_SCALE_H_
#define _VIDEO_SCALE_H_

#include "gstLinkable.h"
#include "noncopyable.h"

// forward declarations
class Pipeline;
class _GstElement;

/** 
 *  A filter that scales video to a specified resolution.
 */

class VideoScale : public GstLinkableFilter, public boost::noncopyable
{
    public:
        VideoScale(const Pipeline &pipeline, int width, int height);
        ~VideoScale();
        /// Found by trial and error, neither width nor height may exceed this value
        static const int MAX_SCALE = 2046;

    private:
        _GstElement *sinkElement() { return videoscale_; }
        _GstElement *srcElement() { return capsfilter_; }

        const Pipeline &pipeline_;
        _GstElement *videoscale_;
        _GstElement *capsfilter_;
};

#endif //_VIDEO_SCALE_H_

