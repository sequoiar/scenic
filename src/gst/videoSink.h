// videoSink.h
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

#ifndef _VIDEO_SINK_H_
#define _VIDEO_SINK_H_

#include "gstLinkable.h"

class _GtkWidget;
class _GdkEventExpose;
class _GdkEventKey;
class _GdkEventScroll;
class _GstElement;

class VideoSink
    : public GstLinkableSink
{
    public:
        VideoSink()
            : sink_(0), window_(0) {};
        ~VideoSink(){};
        void showWindow();
        void makeFullscreen() { makeFullscreen(window_); }
        void makeUnfullscreen() { makeUnfullscreen(window_); }

        _GstElement *sinkElement() { return sink_; }
        
    protected:
        _GstElement *sink_;
        _GtkWidget *window_;
        void destroySink();
        static int expose_cb(_GtkWidget *widget, _GdkEventExpose *event, void *data);
        void makeWindowBlack();
        static void makeFullscreen(_GtkWidget *widget);
        static void makeUnfullscreen(_GtkWidget *widget);
        static void toggleFullscreen(_GtkWidget *widget);

    private:

        VideoSink(const VideoSink&);     //No Copy Constructor
        VideoSink& operator=(const VideoSink&);     //No Assignment Operator
};


class XvImageSink
    : public VideoSink
{
    public:
        XvImageSink() {};
        ~XvImageSink();
        void init();

    private:
        static int key_press_event_cb(_GtkWidget *widget, _GdkEventKey *event,
                void *data);

        XvImageSink(const XvImageSink&);     //No Copy Constructor
        XvImageSink& operator=(const XvImageSink&);     //No Assignment Operator
};


class XImageSink
: public VideoSink
{
    public: 
        XImageSink() : colorspc_(0) {};
        ~XImageSink();
        void init();
        // FIXME: need to implement this support in ximagesink
        void makeFullscreen() {}
        void makeUnfullscreen() {}

        _GstElement *sinkElement() { return colorspc_; }
    private:
        _GstElement *colorspc_;
        XImageSink(const XImageSink&);     //No Copy Constructor
        XImageSink& operator=(const XImageSink&);     //No Assignment Operator
};



#endif //_VIDEO_SINK_H_

