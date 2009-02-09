/* videoConfig.h
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _VIDEO_CONFIG_H_
#define _VIDEO_CONFIG_H_

#include <string>

class VideoSource;
class VideoSink;

class VideoSourceConfig
{
    public:
        //* for a simple source */
        explicit VideoSourceConfig(const std::string &source__, const int bitrate__) 
            : source_(source__), bitrate_(bitrate__), location_("") {}

        //* for source (remote) w/ location i.e. filename or url */
        VideoSourceConfig(const std::string &source__, const int bitrate__, const std::string &location__)
            : source_(source__), bitrate_(bitrate__), location_(location__)  {}

        //* copy constructor */
        VideoSourceConfig(const VideoSourceConfig& m)
            : source_(m.source_), bitrate_(m.bitrate_), location_(m.location_) {}

        VideoSource* createSource() const;  // factory method

        const char *source() const { return source_.c_str(); }
        int bitrate() const { return bitrate_; }

        bool hasLocation() const { return location_.empty(); }
        bool fileExists() const;
        const char *location() const;

    private:
        const std::string source_;
        const int bitrate_;
        const std::string location_;
        /// No Assignment Operator 
        VideoSourceConfig& operator=(const VideoSourceConfig&);     
};


class VideoSinkConfig 
{
    public:

        explicit VideoSinkConfig(const std::string & sink__, int screenNum = 0)
            : sink_(sink__), screenNum_(screenNum)
        {}

        // copy constructor
        VideoSinkConfig(const VideoSinkConfig & m) 
            : sink_(m.sink_) , screenNum_(m.screenNum_)
        {}

        VideoSink* createSink() const;

    private:

        const std::string sink_;
        const int screenNum_;
};

#endif // _VIDEO_CONFIG_H_

