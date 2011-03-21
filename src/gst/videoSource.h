// videoSource.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#ifndef _VIDEO_SOURCE_H_
#define _VIDEO_SOURCE_H_

#include "noncopyable.h"

#include <string>

class Pipeline;
class VideoSourceConfig;
class _GstElement;

class VideoSource
    : private boost::noncopyable
{
    public:
        virtual ~VideoSource(){};
        virtual std::string srcCaps(unsigned framerateIndex = 0) const;
        void setCapsFilter(const std::string &srcCaps);
        virtual _GstElement *srcElement() { return source_; }

    protected:
        VideoSource(const Pipeline &pipeline, const VideoSourceConfig &config);
        const Pipeline &pipeline_;
        const VideoSourceConfig &config_;
        _GstElement *source_;
        _GstElement *capsFilter_;
        std::string defaultSrcCaps() const;
};

class VideoTestSource
    : public VideoSource
{
    public:
        VideoTestSource(const Pipeline &pipeline, const VideoSourceConfig &config);
        void filterCaps();

    private:
        virtual _GstElement *srcElement() { return capsFilter_; }
};

class VideoFileSource
    : public VideoSource
{
    public:
        VideoFileSource(const Pipeline &pipeline, const VideoSourceConfig &config);

    private:
        ~VideoFileSource();
        virtual _GstElement *srcElement() { return identity_; }      

        // FIXME: maybe just use the queue we acquire?
        _GstElement *identity_;
};

class VideoDvSource
    : public VideoSource
{
    public:
        VideoDvSource(const Pipeline &pipeline, const VideoSourceConfig &config);

    private:
        ~VideoDvSource();
        
        virtual _GstElement *srcElement() { return dvdec_; }

        _GstElement *queue_, *dvdec_;
};

class VideoV4lSource
    : public VideoSource
{
    public:
        VideoV4lSource(const Pipeline &pipeline, const VideoSourceConfig &config);
    private:
        std::string expectedStandard_;
        std::string actualStandard_;
        std::string deviceStr() const;
        std::string srcCaps(unsigned int framerateIndex = 0) const;
        bool willModifyCaptureResolution() const;
        virtual _GstElement *srcElement() { return capsFilter_; }
};



class VideoDc1394Source
    : public VideoSource
{
    public:
        VideoDc1394Source(const Pipeline &pipeline, const VideoSourceConfig &config);
    private:
        std::string srcCaps(unsigned int framerateIndex = 0) const;
        virtual _GstElement *srcElement() { return capsFilter_; }
};

#endif //_VIDEO_SOURCE_H_
