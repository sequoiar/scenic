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
VideoConfig::VideoConfig(const std::string &source__, const std::string &codec__,
    const std::string &remoteHost__, int port__)
    : MediaConfig(source__, codec__, remoteHost__, port__)
{
    // empty
}


// for sender (remote)
VideoConfig::VideoConfig(const std::string &source__,
    const std::string &location__,
    const std::string &codec__,
    const std::string &remoteHost__,
    int port__)
    : MediaConfig(source__, location__, codec__, remoteHost__, port__)
{
    // empty
}


// for sender (local)
VideoConfig::VideoConfig(const std::string &source__)
    : MediaConfig(source__)
{
    // empty
}


// for sender (local)
VideoConfig::VideoConfig(const std::string &source__, const std::string &location__)
    : MediaConfig(source__, location__)
{
    // empty
}


// for receiver
VideoConfig::VideoConfig(const std::string &codec__, int port__)
    : MediaConfig(codec__, port__)
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


