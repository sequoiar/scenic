/*
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

#include <string>
#include "video_flip.h"
#include "gst_linkable.h"
#include "pipeline.h"

VideoFlip::VideoFlip(const Pipeline &pipeline, const std::string &flipMethod) :
    identity_(0),
    colorspace_(0),
    videoflip_(0)
{
    LOG_DEBUG("using flip method " << flipMethod);
    // We avoid using the flip method chain if possible since it may cause extra
    // colourspace conversions
    if (flipMethod != "none")
    {
        colorspace_ = pipeline.makeElement("ffmpegcolorspace", NULL);
        videoflip_ = pipeline.makeElement("videoflip", NULL);
        gstlinkable::link(colorspace_, videoflip_);
        gst_util_set_object_arg (G_OBJECT(videoflip_), "method", flipMethod.c_str());
    }
    else
    {
        identity_ = pipeline.makeElement("identity", NULL);
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
