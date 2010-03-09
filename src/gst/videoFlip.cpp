/* videoFlip.cpp
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

#include <string>
#include <map>
#include "util.h"
#include "videoFlip.h"
#include "pipeline.h"

/// converts flip method to corresponding
int flipMethodToInt(const std::string &flipMethod)
{
    using std::map;
    using std::string;

    static map<string, int> FLIP_METHOD_TO_INT;
    if (FLIP_METHOD_TO_INT.empty())
    {
        FLIP_METHOD_TO_INT["none"] = 0;
        FLIP_METHOD_TO_INT["clockwise"] = 1;
        FLIP_METHOD_TO_INT["rotate-180"] = 2;
        FLIP_METHOD_TO_INT["counterclockwise"] = 3;
        FLIP_METHOD_TO_INT["horizontal-flip"] = 4;
        FLIP_METHOD_TO_INT["vertical-flip"] = 5;
        FLIP_METHOD_TO_INT["upper-left-diagonal"] = 6;
        FLIP_METHOD_TO_INT["upper-right-diagonal"] = 7;
    }

    // this is a hack because if the key isn't found, this map will return 0
    int result = FLIP_METHOD_TO_INT[flipMethod];
    if (flipMethod != "none")
    {
        if (result == 0)
            THROW_ERROR("Unknown flipmethod " << flipMethod);
        return result;
    }
    else
        return result;
}

VideoFlip::VideoFlip(const Pipeline &pipeline, const std::string &flipMethod) : 
    pipeline_(pipeline),
    colorspace_(pipeline_.makeElement("ffmpegcolorspace", NULL)),
    videoflip_(pipeline_.makeElement("videoflip", NULL))
{
    LOG_DEBUG("using flip method " << flipMethod);
    g_object_set(G_OBJECT(videoflip_), "method", flipMethodToInt(flipMethod), NULL);
    gstlinkable::link(colorspace_, videoflip_);    
}

/// Destructor 
VideoFlip::~VideoFlip()
{
    pipeline_.remove(&videoflip_);
    pipeline_.remove(&colorspace_);
}

