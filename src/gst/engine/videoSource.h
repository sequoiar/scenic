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

#include "gstLinkable.h"

#include <gst/gstclock.h>

class VideoSourceConfig;
class _GstElement;
class _GstPad;

class VideoSource
    : public GstLinkableSource
{
    public:
        ~VideoSource();
        virtual void init();
        virtual std::string srcCaps() const;
        void setCapsFilter(const std::string &srcCaps);

    protected:
        explicit VideoSource(const VideoSourceConfig &config);
        const VideoSourceConfig &config_;
        _GstElement *source_;
        _GstElement *capsFilter_;
        /*
        static const int WIDTH = 720;
        static const int HEIGHT = 480;
        static const int PIX_ASPECT_NUM = 10;
        static const int PIX_ASPECT_DENOM = 11;
        */
        static const int WIDTH = 640;
        static const int HEIGHT = 480;
        std::string defaultSrcCaps() const;

    private:
        _GstElement *srcElement() { return source_; }
        VideoSource(const VideoSource&);     ///No Copy Constructor
        VideoSource& operator=(const VideoSource&);     ///No Assignment Operator
};

class VideoTestSource
    : public VideoSource
{
    public:
        explicit VideoTestSource(const VideoSourceConfig &config);
        void filterCaps();

    private:
        ~VideoTestSource();
        _GstElement *srcElement() { return capsFilter_; }
        void init();

        /// No Copy Constructor
        VideoTestSource(const VideoTestSource&);     
        /// No Assignment Operator
        VideoTestSource& operator=(const VideoTestSource&);     
};

class VideoFileSource
    : public VideoSource
{
    public:
        explicit VideoFileSource(const VideoSourceConfig &config);

    private:
        ~VideoFileSource();
        _GstElement *srcElement() { return identity_; }      
        void init();

        // FIXME: get rid of this
        _GstElement *identity_;

        /// No Copy Constructor
        VideoFileSource(const VideoFileSource&);     
        /// No Assignment Operator
        VideoFileSource& operator=(const VideoFileSource&);     
};

class VideoDvSource
    : public VideoSource
{
    public:
        explicit VideoDvSource(const VideoSourceConfig &config);

    private:
        ~VideoDvSource();
        
        _GstElement *srcElement() { return dvdec_; }
        void init();

        _GstElement *queue_, *dvdec_;
        VideoDvSource(const VideoDvSource&);     //No Copy Constructor
        VideoDvSource& operator=(const VideoDvSource&);     //No Assignment Operator
};

class VideoV4lSource
    : public VideoSource
{
    public:
        explicit VideoV4lSource(const VideoSourceConfig &config)
            : VideoSource(config), expectedStandard_("NTSC") {}
    private:
        void init();
        std::string expectedStandard_;
        _GstElement *srcElement() { return capsFilter_; }
        VideoV4lSource(const VideoV4lSource&);     //No Copy Constructor
        VideoV4lSource& operator=(const VideoV4lSource&);     //No Assignment Operator
};

#endif //_VIDEO_SOURCE_H_

