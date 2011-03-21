
/* textOverlay.h
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

#ifndef _TEXT_OVERLAY_H_
#define _TEXT_OVERLAY_H_

#include "noncopyable.h"
#include <string>

// forward declarations
class Pipeline;
class _GstElement;

/** 
 *  A filter that overlays text on a video
 */

class TextOverlay: private boost::noncopyable
{
    public:
        TextOverlay(const Pipeline &pipeline, const std::string &text);
        _GstElement *sinkElement() { return textoverlay_; }
        _GstElement *srcElement() { return textoverlay_; }


    private:
        const Pipeline &pipeline_;
        _GstElement *textoverlay_;
};

#endif //_TEXT_OVERLAY_H_

