
// videoConfig.h
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
 *      Class for video parameter objects.
 *
 */

#ifndef _VIDEO_CONFIG_H_
#define _VIDEO_CONFIG_H_

#include <string>
#include "mediaConfig.h"

class VideoConfig : public MediaConfig
{
    public:
        VideoConfig(std::string source, 
                std::string codec = "", std::string remoteHost = "", int port = 0); // used by sender
        VideoConfig(int port = 0);     // used by receiver

        const std::string & source() const;
        
    private:
        const std::string source_;
};

#endif // _VIDEO_CONFIG_H_

