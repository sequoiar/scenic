
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

class VideoSource;
class VideoSink;

class VideoConfig
{
    public:
        VideoConfig(const std::string &source__) : source_(source__), location_("") {}

        // for sender (remote) w/ location i.e. filename or url
        VideoConfig(const std::string &source__, const std::string &location__)
            : source_(source__), location_(location__)  {}

        // copy constructor
        VideoConfig(const VideoConfig& m)
            : source_(m.source_), location_(m.location_) {}

        VideoSource* createSource() const;  // factory method
        VideoSink* createSink() const;  // factory method

        const char *source() const { return source_.c_str(); }

        bool fileExists() const;
        bool sanityCheck() const;
        const char *location() const;

    private:
        
        const std::string source_;
        const std::string location_;

        VideoConfig& operator=(const VideoConfig&);     //No Assignment Operator
};


class VideoReceiverConfig 
{
    public:

        VideoReceiverConfig(const std::string & sink__)
            : sink_(sink__)
        {}

        // copy constructor
        VideoReceiverConfig(const VideoReceiverConfig & m) 
            : sink_(m.sink_) 
        {}

        VideoSink* createSink() const;

    private:

        const std::string sink_;
};

#endif // _VIDEO_CONFIG_H_

