// 
// videoSession.cpp
//
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
#include "videoSession.h"

VideoSession::VideoSession(std::string source, std::string codec, std::string remoteHost, int port) 
: MediaSession(codec, remoteHost, port), source_(source)
{
    // empty
}

    

VideoSession::VideoSession(int port) 
    : MediaSession("", "", port), source_("")
{
    // empty
}

const std::string & VideoSession::source() const
{
    return source_;
}
