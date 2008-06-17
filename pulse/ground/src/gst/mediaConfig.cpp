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

MediaConfig::MediaConfig(std::string codec, std::string remoteHost, int port) 
: codec_(codec), remoteHost_(remoteHost), port_(port)
{
    // empty
}



const std::string & MediaConfig::codec() const
{
    return codec_;
}



const std::string & MediaConfig::remoteHost() const
{
    return remoteHost_;
}


        
const int MediaConfig::port() const
{
    return port_;
}

