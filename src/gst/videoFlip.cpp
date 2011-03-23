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
#include "videoFlip.h"
#include "gstLinkable.h"
#include "pipeline.h"

VideoFlip::VideoFlip(const Pipeline &pipeline, const std::string &flipMethod) : 
    pipeline_(pipeline),
    identity_(0),
    colorspace_(0),
    videoflip_(0)
{
    LOG_DEBUG("using flip method " << flipMethod);
    // We avoid using the flip method chain if possible since it may cause extra
    // colourspace conversions
    if (flipMethod != "none")
    {
        colorspace_ = pipeline_.makeElement("ffmpegcolorspace", NULL);
        videoflip_ = pipeline_.makeElement("videoflip", NULL);
        gstlinkable::link(colorspace_, videoflip_);
        gst_util_set_object_arg (G_OBJECT(videoflip_), "method", flipMethod.c_str());
    }
    else
    {
        identity_ = pipeline_.makeElement("identity", NULL);
        g_object_set(identity_, "silent", TRUE, NULL);
    }
}

GstElement * VideoFlip::sinkElement()
{
    if (identity_)
        return identity_;
    else
        return colorspace_;
}

GstElement *VideoFlip::srcElement()
{
    if (identity_)
        return identity_;
    else
        return videoflip_;
}
