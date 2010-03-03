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
#include "videoSize.h"

#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include "videoConfig.h"
#include "videoSource.h"
#include "videoSink.h"
#include "videoFlip.h"
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


VideoSourceConfig::VideoSourceConfig(const boost::program_options::variables_map &options) : 
    source_(options["videosource"].as<std::string>()), 
    bitrate_(options["videobitrate"].as<int>()), 
    quality_(options["videoquality"].as<int>()), 
    deviceName_(options["videodevice"].as<std::string>()),
    location_(options["videolocation"].as<std::string>()), 
    cameraNumber_(options["camera-number"].as<int>()),
    GUID_(fromString<unsigned long long>(options["camera-guid"].as<std::string>(), std::hex)),
    framerate_(options["framerate"].as<int>()),
    captureWidth_(options["width"].as<int>()),
    captureHeight_(options["height"].as<int>()),
    grayscale_(options["grayscale"].as<bool>()),
    pictureAspectRatio_(options["aspect-ratio"].as<std::string>())
{}


VideoSource * VideoSourceConfig::createSource(Pipeline &pipeline) const
{
    // FIXME: should derived class specific arguments just be passed in here to their constructors?
    if (source_ == "videotestsrc")
        return new VideoTestSource(pipeline, *this);
    else if (source_ == "v4l2src")
        return new VideoV4lSource(pipeline, *this);
    else if (source_ == "v4lsrc")
        return new VideoV4lSource(pipeline, *this);
    else if (source_ == "dv1394src")
        return new VideoDvSource(pipeline, *this);
    else if (source_ == "filesrc")
        return new VideoFileSource(pipeline, *this);
    else if (source_ == "dc1394src")
        return new VideoDc1394Source(pipeline, *this);
    else 
        THROW_ERROR(source_ << " is an invalid source!");

    LOG_DEBUG("Video source options: " << source_ << ", bitrate: " << bitrate_ << ", location: " 
            << location_ << ", device: " << deviceName_);
    return 0;
}


unsigned VideoSourceConfig::captureWidth() const
{
    return captureWidth_;
}


unsigned VideoSourceConfig::captureHeight() const
{
    return captureHeight_;
}


std::string VideoSourceConfig::pictureAspectRatio() const
{
    /// FIXME: have this be settable
    return pictureAspectRatio_;
}


bool VideoSourceConfig::forceGrayscale() const
{
    return grayscale_;
}


bool VideoSourceConfig::locationExists() const
{
    return boost::filesystem::exists(location_);
}



bool VideoSourceConfig::deviceExists() const
{
    return boost::filesystem::exists(deviceName_);
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
    Dc1394::listCameras();
    v4l2util::listCameras();
    return 0;
}

void VideoSourceConfig::setStandard(const std::string &device, std::string standard)
{
    // convert to upper case
    std::transform(standard.begin(), standard.end(), standard.begin(), ::toupper);
    v4l2util::setStandard(device, standard);
}


void VideoSourceConfig::setInput(const std::string &device, int input)
{
    v4l2util::setInput(device, input);
}

std::string VideoSourceConfig::pixelAspectRatio() const
{
    return calculatePixelAspectRatio(captureWidth_, captureHeight_, pictureAspectRatio_);
}

std::string VideoSourceConfig::calculatePixelAspectRatio(int width, int height, const std::string &pictureAspectRatio)
{
// Reference:
// http://en.wikipedia.org/wiki/Pixel_aspect_ratio#Pixel_aspect_ratios_of_common_video_formats

    using std::map;
    using std::string;
    typedef map < string, map < string, string > > Table;

    static Table PIXEL_ASPECT_RATIO_TABLE;
    // only does this once
    if (PIXEL_ASPECT_RATIO_TABLE.empty())
    {
        // PAL
        PIXEL_ASPECT_RATIO_TABLE["720x576"]["4:3"] = 
            PIXEL_ASPECT_RATIO_TABLE["704x576"]["4:3"] = "59/54";

        PIXEL_ASPECT_RATIO_TABLE["704x576"]["16:9"] = 
            PIXEL_ASPECT_RATIO_TABLE["352x288"]["16:9"] = "118/81";

        // NTSC
        PIXEL_ASPECT_RATIO_TABLE["720x480"]["4:3"] = 
            PIXEL_ASPECT_RATIO_TABLE["704x480"]["4:3"] = "10/11";

        PIXEL_ASPECT_RATIO_TABLE["704x480"]["16:9"] = 
            PIXEL_ASPECT_RATIO_TABLE["352x240"]["16:9"] = "40/33";

        /// Misc. Used by us
        PIXEL_ASPECT_RATIO_TABLE["768x480"]["4:3"] = "6/7";
        PIXEL_ASPECT_RATIO_TABLE["640x480"]["4:3"] = "1/1"; // square pixels
    }
    std::stringstream resolution;
    resolution << width << "x" << height;
    std::string result = PIXEL_ASPECT_RATIO_TABLE[resolution.str()][pictureAspectRatio];
    if (result == "")
        result = "1/1"; // default to square pixels

    LOG_DEBUG("Pixel-aspect-ratio is " << result);
    return result;
}


VideoSinkConfig::VideoSinkConfig(const boost::program_options::variables_map &options) : 
    sink_(options["videosink"].as<std::string>()), 
    screenNum_(options["screen"].as<int>()), 
    doDeinterlace_(options["deinterlace"].as<bool>()), 
    sharedVideoId_(options["shared-video-id"].as<std::string>()),
    /// if display-resolution is not specified, default to capture-resolution
    displayWidth_(std::min(static_cast<int>(options.count("display-width") ? 
                    options["display-width"].as<int>() : options["width"].as<int>()), VideoScale::MAX_SCALE)),
    displayHeight_(std::min(static_cast<int>(options.count("display-height") ? 
                    options["display-height"].as<int>() : options["height"].as<int>()), VideoScale::MAX_SCALE)),
    flipMethod_(options["flip-video"].as<std::string>()),
    xid_(options["x-window-id"].as<unsigned long>())
{}


bool VideoSinkConfig::resolutionIsInverted() const
{
    return flipMethod_ == "clockwise" or flipMethod_ == "counterclockwise"
        or flipMethod_ == "upper-left-diagonal" or flipMethod_ == "upper-right-diagonal";
}

int VideoSinkConfig::effectiveDisplayWidth() const
{
    if (resolutionIsInverted())
        return displayHeight_;
    else
        return displayWidth_;
}


int VideoSinkConfig::effectiveDisplayHeight() const
{
    if (resolutionIsInverted())
        return displayWidth_;
    else
        return displayHeight_;
}


VideoSink * VideoSinkConfig::createSink(Pipeline &pipeline) const
{
    if (sink_ == "xvimagesink")
        return new XvImageSink(pipeline, effectiveDisplayWidth(), effectiveDisplayHeight(), screenNum_, xid_);
    else if (sink_ == "ximagesink")
        return new XImageSink(pipeline);
#ifdef CONFIG_GL
    else if (sink_ == "glimagesink")
        return new GLImageSink(pipeline, effectiveDisplayWidth(), effectiveDisplayHeight(), screenNum_, xid_);
#endif
    else if (sink_ == "sharedvideosink")
        return new SharedVideoSink(pipeline, effectiveDisplayWidth(), effectiveDisplayHeight(), sharedVideoId_);
    else
        THROW_ERROR(sink_ << " is an invalid sink");

    LOG_DEBUG("Video sink " << sink_ << " built"); 
    return 0;
}


VideoScale* VideoSinkConfig::createVideoScale(Pipeline &pipeline) const
{
    return new VideoScale(pipeline, displayWidth_, displayHeight_);
}


VideoFlip* VideoSinkConfig::createVideoFlip(Pipeline &pipeline) const
{
    return new VideoFlip(pipeline, flipMethod_);
}

