/* videoConfig.cpp
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
#include "mapMsg.h"

#include <fstream>
#include "videoConfig.h"
#include "videoSource.h"
#include "videoSink.h"
#include "videoScale.h"
#include "sharedVideoSink.h"

#ifdef CONFIG_GL
#include "glVideoSink.h"
#endif

#include "dc1394.h"
#include "v4l2util.h"


template <class T>
T fromString(const std::string& s, 
                 std::ios_base& (*f)(std::ios_base&))
{
    T t;
    std::istringstream iss(s);
    if ((iss >> f >> t).fail())
        THROW_CRITICAL("Could not convert string " << s << " to hex");
    return t;
}


VideoSourceConfig::VideoSourceConfig(MapMsg &msg) : 
    source_(msg["source"]), 
    bitrate_(msg["bitrate"]), 
    quality_(msg["quality"]), 
    deviceName_(msg["device"]),
    location_(msg["location"]), 
    cameraNumber_(msg["camera-number"]),
    GUID_(fromString<unsigned long long>(msg["camera-guid"], std::hex)),
    framerate_(msg["framerate"])
{}


VideoSource * VideoSourceConfig::createSource() const
{
    // FIXME: should derived class specific arguments just be passed in here to their constructors?
    if (source_ == "videotestsrc")
        return new VideoTestSource(*this);
    else if (source_ == "v4l2src")
        return new VideoV4lSource(*this);
    else if (source_ == "v4lsrc")
        return new VideoV4lSource(*this);
    else if (source_ == "dv1394src")
        return new VideoDvSource(*this);
    else if (source_ == "filesrc")
        return new VideoFileSource(*this);
    else if (source_ == "dc1394src")
        return new VideoDc1394Source(*this);
    else 
        THROW_ERROR(source_ << " is an invalid source!");

    LOG_DEBUG("Video source options: " << source_ << ", bitrate: " << bitrate_ << ", location: " 
            << location_ << ", device: " << deviceName_);
    return 0;
}


bool VideoSourceConfig::locationExists() const
{
    return fileExists(location_);
}



bool VideoSourceConfig::deviceExists() const
{
    return fileExists(deviceName_);
}


const char* VideoSourceConfig::location() const
{
    return location_.c_str();
}

const char* VideoSourceConfig::deviceName() const
{
    return deviceName_.c_str();
}


int VideoSourceConfig::listCameras()
{
    DC1394::listCameras();
    v4l2util::listCameras();
    return 0;
}


VideoSinkConfig::VideoSinkConfig(MapMsg &msg) : 
    sink_(msg["sink"]), 
    screenNum_(msg["screen"]), 
    doDeinterlace_(msg["deinterlace"]), 
    sharedVideoId_(msg["shared-video-id"]),
    width_(msg["width"]),
    height_(msg["height"])
{}


VideoSink * VideoSinkConfig::createSink(int width, int height) const
{
    if (sink_ == "xvimagesink")
        return new XvImageSink(width, height, screenNum_);
    else if (sink_ == "ximagesink")
        return new XImageSink();
#ifdef CONFIG_GL
    else if (sink_ == "glimagesink")
        return new GLImageSink(screenNum_);
#endif
    else if (sink_ == "sharedvideosink")
        return new SharedVideoSink(sharedVideoId_);
    else
        THROW_ERROR(sink_ << " is an invalid sink");

    LOG_DEBUG("Video sink " << sink_ << " built"); 
    return 0;
}


VideoScale* VideoSinkConfig::createVideoScale() const
{
    return new VideoScale(width_, height_);
}


bool VideoSinkConfig::hasCustomResolution() const
{
    return width_ != videosize::WIDTH or height_ != videosize::HEIGHT;
}

