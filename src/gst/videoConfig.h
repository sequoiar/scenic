
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

        VideoConfig(const std::string & source,
        const std::string & codec,
        const std::string & remoteHost,
        int port);             // used by sender

        VideoConfig(const std::string & source,
        const std::string & location,
        const std::string & codec,
        const std::string & remoteHost,
        int port);             // used by sender

        VideoConfig(const std::string & codec, int port); // used by receiver
        explicit VideoConfig(const std::string & source); // used by local sender
        explicit VideoConfig(const std::string & source, const std::string & location); // used by local sender

        VideoSource* createSource() const;  // factory method

        const char* location() const;

        const bool has_h264() const { return codec_ == "h264"; }
    private:
        const std::string location_;
};

#endif // _VIDEO_CONFIG_H_

