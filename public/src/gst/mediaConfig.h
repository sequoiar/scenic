// mediaConfig.h
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
 *      Immutable base class for media parameter objects.
 *
 */

#ifndef _MEDIA_CONFIG_H_
#define _MEDIA_CONFIG_H_

#include <string>

class MediaConfig
{
    public:
        // remote sender
        MediaConfig(const std::string & source__,
        const std::string & codec__,
        const std::string & remoteHost__,
        int port__)
            : source_(source__), location_(""), codec_(codec__), 
            remoteHost_(remoteHost__), port_(port__) {}

        // remote sender (with location)
        MediaConfig(const std::string & source__,
        const std::string & location__,
        const std::string & codec__,
        const std::string & remoteHost__,
        int port__)
            : source_(source__), location_(location__), codec_(codec__),
            remoteHost_(remoteHost__), port_(port__) {}

        // receiver
        MediaConfig(const std::string &codec__, int port__)
            : source_(""), location_(""), codec_(codec__), 
            remoteHost_(""), port_(port__) {}

        // local sender
        explicit MediaConfig(const std::string &source__)
            : source_(source__), location_(""), codec_(""), 
            remoteHost_(""), port_(0) {}

        // local sender (w/ location)
        explicit MediaConfig(const std::string &source__, const std::string &location__) 
            : source_(source__), location_(location__), codec_(""), remoteHost_(""), port_(0) {}

        // copy constructor
        MediaConfig(const MediaConfig &m) : 
            source_(m.source_), location_(m.location_), codec_(m.codec_),
            remoteHost_(m.remoteHost_), port_(m.port_) {}

        const char *source() const { return source_.c_str(); }
        const char *location() const;

        const char *codec() const { return codec_.c_str(); }
        const char *remoteHost() const { return remoteHost_.c_str(); }
        int port() const { return port_; }
        bool isNetworked() const { return port_ != 0; }
        bool hasCodec() const { return !codec_.empty(); }
        bool fileExists() const;

        virtual ~MediaConfig() {};

    protected:
        virtual bool sanityCheck() const = 0;

        const std::string source_;
        const std::string location_;
        const std::string codec_;
        const std::string remoteHost_;
        const int port_;
    private:
        MediaConfig& operator=(const MediaConfig&);     //No Assignment Operator
};

#endif // _MEDIA_CONFIG_H_

