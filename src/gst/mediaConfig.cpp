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

MediaConfig::MediaConfig(std::string codec, int port)   // receiver
:codec_(codec), remoteHost_(""), port_(port)
{
    // empty
}

MediaConfig::MediaConfig(std::string source, std::string codec, std::string remoteHost, int port):source_(source), codec_(codec), remoteHost_(remoteHost), port_(port)  // remote sender
{
    // empty
}

MediaConfig::MediaConfig(std::string source)    // local sender
:  source_(source), remoteHost_(""), port_(0)
{
    // empty
}

const char *MediaConfig::source() const const
{
    return source_.c_str();
}

const char *MediaConfig::codec() const const
{
    return codec_.c_str();
}

const char *MediaConfig::remoteHost() const const
{
    return remoteHost_.c_str();
}

const int MediaConfig::port() const const
{
    return port_;
}

const bool MediaConfig::isNetworked() const const
{
    return port_ != 0;
}

const bool MediaConfig::hasCodec() const const
{
    return codec_.empty();
}
