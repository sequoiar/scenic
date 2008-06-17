
// audioSession.cpp
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
 *      Class for audio parameter objects.
 *
 */

#include <string>
#include "audioSession.h"

AudioSession::AudioSession(int numChannels, std::string codec, std::string remoteHost, int port) 
: MediaSession(codec, remoteHost, port),  numChannels_(numChannels)
{
    // empty
}



AudioSession::AudioSession(int numChannels, std::string codec, int port) 
: MediaSession(codec, "localhost", port),  numChannels_(numChannels)
{
    // empty
}


const int AudioSession::numChannels() const
{
    return numChannels_;
}

