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

#include "util.h"
#include "videoScale.h"
#include <boost/assign.hpp>
#include <sstream>
#include "pipeline.h"


/** Constructor sets width and height */
VideoScale::VideoScale(int width, int height) : 
    videoscale_(Pipeline::Instance()->makeElement("videoscale", NULL)),
    capsfilter_(Pipeline::Instance()->makeElement("capsfilter", NULL))
{
    using namespace boost::assign;
    using std::string;
    using std::vector;
    GstCaps *videoCaps = 0; 
    GstCaps *tempCaps;
    
    static const vector<string> FORMATS = 
        list_of<string>("x-raw-yuv")("x-raw-rgb")("x-raw-gray");

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
    gst_caps_unref(videoCaps);
}

/// Destructor 
VideoScale::~VideoScale()
{
    Pipeline::Instance()->remove(&capsfilter_);
    Pipeline::Instance()->remove(&videoscale_);
}

/// FIXME: merge some of this?
int VideoScale::getWidth() const
{
    int width;

    // Get caps directly from capsfilter so we're sure they're accurate
    GstCaps *caps;
    g_object_get(capsfilter_, "caps", &caps, NULL);

    // get fixed caps structure, then get width out of it 
    if (not gst_structure_get_int(gst_caps_get_structure(caps, 0), "width", &width))
        THROW_ERROR("Could not get width from caps");

    LOG_DEBUG("Got width " << width);
    return width;  
}


int VideoScale::getHeight() const
{
    int height;

    // Get caps directly from capsfilter so we're sure they're accurate
    GstCaps *caps;
    g_object_get(capsfilter_, "caps", &caps, NULL);

    // get fixed caps structure, then get width out of it 
    if (not gst_structure_get_int(gst_caps_get_structure(caps, 0), "height", &height))
        THROW_ERROR("Could not get height from caps");
    LOG_DEBUG("Got height " << height);
    return height;  
}

