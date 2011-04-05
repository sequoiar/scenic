/* videoScale.cpp
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

#include <gst/gst.h>
#include "util.h"
#include "videoScale.h"
#include <boost/assign.hpp>
#include <sstream>
#include "pipeline.h"
#include "gstLinkable.h"

const int VideoScale::MAX_SCALE;

/** Constructor sets width and height */
VideoScale::VideoScale(const Pipeline &pipeline, int width, int height) : 
    pipeline_(pipeline),
    videoscale_(pipeline_.makeElement("videoscale", NULL)),
    capsfilter_(pipeline_.makeElement("capsfilter", NULL))
{
    using namespace boost::assign;
    using std::string;
    using std::vector;
    GstCaps *videoCaps = 0; 
    GstCaps *tempCaps;
    
    static const vector<string> FORMATS = 
        list_of<string>("x-raw-gray")("x-raw-yuv")("x-raw-rgb");

    for (vector<string>::const_iterator format = FORMATS.begin();
            format != FORMATS.end(); ++format)
    {
        std::ostringstream capsStr;

        capsStr << "video/" << *format << ", width=" << width 
            << ", height=" << height; 
        tempCaps = gst_caps_from_string(capsStr.str().c_str());
        if (not videoCaps)
            videoCaps = tempCaps;
        else
            gst_caps_append(videoCaps, tempCaps);
    }

    LOG_DEBUG("Setting caps to " << gst_caps_to_string(videoCaps));
    g_object_set(capsfilter_, "caps", videoCaps, NULL);
    gstlinkable::link(videoscale_, capsfilter_);

    // Don't need to unref tempCaps, this happens every time we append it
    // to videoCaps
    if (videoCaps)
        gst_caps_unref(videoCaps);
}

/// Destructor 
VideoScale::~VideoScale()
{
    pipeline_.remove(&capsfilter_);
    pipeline_.remove(&videoscale_);
}

