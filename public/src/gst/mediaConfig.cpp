// mediaConfig.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
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
 *      Base class for media parameter objects.
 *
 */

#include <string>
#include "mediaConfig.h"
#include "logWriter.h"

MediaConfig::MediaConfig(const std::string &codec_param, int port_param)   // receiver
    : source_(""), location_(""), codec_(codec_param), remoteHost_(""), port_(port_param)
{
    // empty
}


MediaConfig::MediaConfig(const std::string &source_param, const std::string &codec_param,
    const std::string &remoteHost_param, int port_param)
    : source_(source_param), location_(""), codec_(codec_param), remoteHost_(remoteHost_param), port_(port_param) // remote sender
{
    // empty
}


MediaConfig::MediaConfig(const std::string &source_param, const std::string &location_param, const std::string &codec_param,
    const std::string &remoteHost_param, int port_param)
    : source_(source_param), location_(location_param), codec_(codec_param), remoteHost_(remoteHost_param), port_(port_param) // remote sender
{
    // empty
}


MediaConfig::MediaConfig(const std::string &source_param)    // local sender
    : source_(source_param), location_(""), codec_(""), remoteHost_(""), port_(0)
{
    // empty
}


MediaConfig::MediaConfig(const std::string &source_param, const std::string &location_param)    // local sender
    : source_(source_param), location_(location_param), codec_(""), remoteHost_(""), port_(0)
{
    // empty
}


// FIXME: not every mediaconfig has a file
bool MediaConfig::fileExists() const
{
    if (location_.empty())
        return false;

    FILE *file;
    file = fopen(location(), "r");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    else
        return false;
}


const char* MediaConfig::location() const
{
    if (!location_.empty())
        return location_.c_str();
    else {
        LOG("No location specified", ERROR);
        return NULL;
    }
}


