//
// videoConfig.cpp // // Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

/** \file
 *      Class for video parameter objects.
 *
 */

#include <string>
#include <iostream>
#include "logWriter.h"
#include "videoConfig.h"
#include "videoSource.h"


// for sender (remote)
VideoConfig::VideoConfig(const std::string &source, const std::string &codec,
    const std::string &remoteHost, int port)
    : MediaConfig(source, codec, remoteHost, port), location_("")
{
    // empty
}


// for sender (remote)
VideoConfig::VideoConfig(const std::string &source,
    const std::string &location,
    const std::string &codec,
    const std::string &remoteHost,
    int port)
    : MediaConfig(source, codec, remoteHost, port), location_(location)
{
    // empty
}


// for sender (local)
VideoConfig::VideoConfig(const std::string &source)
    : MediaConfig(source), location_("")
{
    // empty
}


// for sender (local)
VideoConfig::VideoConfig(const std::string &source, const std::string &location)
    : MediaConfig(source), location_(location)
{
    // empty
}


// for receiver
VideoConfig::VideoConfig(const std::string &codec, int port)
    : MediaConfig(codec, port), location_("")
{
    // empty
}


VideoSource * VideoConfig::createSource() const
{
    if (source_ == "videotestsrc")
        return new VideoTestSource(*this);
    else if (source_ == "v4l2src")
        return new VideoV4lSource(*this);
    else if (source_ == "dv1394src")
        return new VideoDvSource(*this);
    else if (source_ == "filesrc")
        return new VideoFileSource(*this);
    else {
        LOG("Invalid source!", ERROR);
        return 0;
    }
}


const char* VideoConfig::location() const
{
    if (!location_.empty())
        return location_.c_str();
    else{
        LOG("No location specified", ERROR);
        return "";
    }
}


