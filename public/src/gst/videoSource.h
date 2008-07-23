// videoSource.h
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

#ifndef _VIDEO_SOURCE_H_
#define _VIDEO_SOURCE_H_

#include <cassert>
#include "gstBase.h"

class VideoConfig;

class VideoSource : public GstBase
{
    public:
        virtual ~VideoSource();
        void init();
        virtual void sub_init() = 0;
        virtual GstElement *output() { return source_; }
    protected:
        VideoSource(const VideoConfig &config);
        const VideoConfig &config_;
        GstElement *source_;
        static gboolean base_callback(GstClock *clock, GstClockTime time, GstClockID id, gpointer user_data);
        virtual gboolean callback() { return FALSE; }
};

class VideoTestSource : public VideoSource
{
    public:
        VideoTestSource(const VideoConfig &config) : VideoSource(config) {}
        ~VideoTestSource();
        void sub_init();
        gboolean callback();

    private:
       GstClockID clockId_;
};

class VideoFileSource : public VideoSource
{
    public:
        VideoFileSource(const VideoConfig &config) : VideoSource(config) {}
        ~VideoFileSource(){};
        void sub_init();
};

class VideoDvSource : public VideoSource
{
    public:
        VideoDvSource(const VideoConfig &config);
        GstElement *output() { return dvdec_; }
        void sub_init();
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, void *data);

    private:
        GstElement *demux_, *queue_, *dvdec_;
};

class VideoV4lSource : public VideoSource
{
    public:
        VideoV4lSource(const VideoConfig &config): VideoSource(config) {} 
        void sub_init(){};
};

#endif //_VIDEO_SOURCE_H_

