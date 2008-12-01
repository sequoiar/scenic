/* videoConfig.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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


/** \file
 *      Class for video parameter objects.
 *
 */

#include <fstream>
#include "logWriter.h"
#include "videoConfig.h"
#include "logWriter.h"
#include "videoSource.h"
#include "videoSink.h"
#include "config.h"
#ifdef CONFIG_GL
#include "glVideoSink.h"
#endif

VideoSource * VideoSourceConfig::createSource() const
{
    if (source_ == "videotestsrc")
        return new VideoTestSource(*this);
    else if (source_ == "v4l2src")
        return new VideoV4lSource(*this);
    else if (source_ == "dv1394src")
        return new VideoDvSource(*this);
    else if (source_ == "filesrc")
        return new VideoFileSource(*this);
    else 
        THROW_ERROR(source_ << " is an invalid source!");
    return 0;
}


bool VideoSourceConfig::fileExists() const
{
    std::fstream in;
    in.open(location_.c_str(), std::fstream::in);
    if (in.fail())
    {
        LOG_DEBUG("File " << location_ << " does not exist");
        return false;
    }

    in.close();
    return true;
}


const char* VideoSourceConfig::location() const
{
    return location_.c_str();
}


VideoSink * VideoSinkConfig::createSink() const
{
    if (sink_ == "xvimagesink")
        return new XvImageSink();
    else if (sink_ == "ximagesink")
        return new XImageSink();
#ifdef CONFIG_GL
    else if (sink_ == "glimagesink")
        return new GLImageSink(screenNum_);
#endif
    else
        THROW_ERROR(sink_ << " is an invalid sink");
    return 0;
}

