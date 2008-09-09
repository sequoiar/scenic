
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

class VideoSource;

class VideoConfig
    : public MediaConfig
{
    public:
        // for sender (remote)
        VideoConfig(const std::string &source__, 
                const std::string &codec__,
                const std::string &remoteHost__, 
                int port__)
            : MediaConfig(source__, codec__, remoteHost__, port__) {}

        // for sender (remote) w/ location
        VideoConfig(const std::string &source__,
                const std::string &location__,
                const std::string &codec__,
                const std::string &remoteHost__,
                int port__)
            : MediaConfig(source__, location__, codec__, remoteHost__, port__) {}

        // for receiver
        VideoConfig(const std::string &codec__, int port__)
            : MediaConfig(codec__, port__) {}

        // used by local sender
        explicit VideoConfig(const std::string &source__)
            : MediaConfig(source__) {}

        // used by local sender w/ file
        VideoConfig(const std::string &source__, const std::string &location__)
            : MediaConfig(source__, location__) {}

        VideoSource* createSource() const;  // factory method

        bool has_h264() const { return codec_ == "h264"; }
};

#endif // _VIDEO_CONFIG_H_

