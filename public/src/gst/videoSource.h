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

#include "gstLinkable.h"

class VideoConfig;
class _GstElement;
class _GstPad;

class VideoSource
    : public GstLinkableSource
{
    public:
        ~VideoSource();
        bool init();

        _GstElement *srcElement() { return source_; }
        virtual void sub_init() = 0;

        //virtual void link_element(_GstElement *sinkElement);

    protected:
        explicit VideoSource(const VideoConfig &config)
            : config_(config), source_(0) {}

        const VideoConfig &config_;
        _GstElement *source_;
        static gboolean base_callback(GstClock *clock, GstClockTime time, GstClockID id,
                                      gpointer user_data);

        virtual gboolean callback() { return FALSE; }
    private:
        VideoSource(const VideoSource&);     //No Copy Constructor
        VideoSource& operator=(const VideoSource&);     //No Assignment Operator
};

class VideoTestSource
    : public VideoSource
{
    public:
        explicit VideoTestSource(const VideoConfig &config)
            : VideoSource(config), clockId_(0) {}
        ~VideoTestSource();
        void sub_init();
        gboolean callback();

    private:
        void toggle_colour();

        GstClockID clockId_;
        static const int BLACK;
        static const int WHITE;

        VideoTestSource(const VideoTestSource&);     //No Copy Constructor
        VideoTestSource& operator=(const VideoTestSource&);     //No Assignment Operator
};

class VideoFileSource
    : public VideoSource
{
    public:
        explicit VideoFileSource(const VideoConfig &config)
            : VideoSource(config), decoder_(0) {}
        ~VideoFileSource();
        _GstElement *srcElement() { return 0; }      // FIXME: HACK
        void sub_init();

        //void link_element(_GstElement *sinkElement);

    private:
        _GstElement *decoder_;
        static void cb_new_src_pad(_GstElement * srcElement, _GstPad * srcPad, gboolean last,
                                   void *data);

        VideoFileSource(const VideoFileSource&);     //No Copy Constructor
        VideoFileSource& operator=(const VideoFileSource&);     //No Assignment Operator
};

class VideoDvSource
    : public VideoSource
{
    public:
        explicit VideoDvSource(const VideoConfig &config) 
            : VideoSource(config), demux_(0), queue_(0), dvdec_(0), dvIsNew_(true) {}

        ~VideoDvSource();
        
        _GstElement *srcElement() { return dvdec_; }
        bool init();
        void sub_init();
        static void cb_new_src_pad(_GstElement * srcElement, _GstPad * srcPad, void *data);

    private:
        _GstElement *demux_, *queue_, *dvdec_;
        bool dvIsNew_;
        VideoDvSource(const VideoDvSource&);     //No Copy Constructor
        VideoDvSource& operator=(const VideoDvSource&);     //No Assignment Operator
};

class VideoV4lSource
    : public VideoSource
{
    public:
        explicit VideoV4lSource(const VideoConfig &config)
            : VideoSource(config) {}
        void sub_init(){};
    private:
        VideoV4lSource(const VideoV4lSource&);     //No Copy Constructor
        VideoV4lSource& operator=(const VideoV4lSource&);     //No Assignment Operator
};

#endif //_VIDEO_SOURCE_H_

