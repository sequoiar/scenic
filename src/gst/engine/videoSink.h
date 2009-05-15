// videoSink.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#include <X11/Xlib.h>
#include "gstLinkable.h"
#include "busMsgHandler.h"

class _GtkWidget;
class _GdkEventExpose;
class _GdkEventKey;
class _GdkEventScroll;
class _GstElement;

class VideoSink : public GstLinkableSink
{
    public:
       VideoSink() : sink_(0) {};
        virtual ~VideoSink(){};
        virtual void init() = 0;
        virtual void makeFullscreen() = 0;
        virtual void makeUnfullscreen() = 0; 
        void destroySink();

    protected:
        void prepareSink();
        _GstElement *sink_;

    private:
       VideoSink(const VideoSink&);     //No Copy Constructor
       VideoSink& operator=(const VideoSink&);     //No Assignment Operator
};

class GtkVideoSink
    : public VideoSink
{
    public:
       GtkVideoSink(int screen_num)
            : window_(0), screen_num_(screen_num) {};
        virtual ~GtkVideoSink(){};
        void makeFullscreen() { makeFullscreen(window_); }
        void makeUnfullscreen() { makeUnfullscreen(window_); }
        void showWindow();

        
    protected:
        _GtkWidget *window_;
        int screen_num_;
        static const unsigned int WIDTH = 640;
        static const unsigned int HEIGHT = 480;

        static void destroy_cb(_GtkWidget * /*widget*/, void *data);
        Window getXWindow();
        static int expose_cb(_GtkWidget *widget, _GdkEventExpose *event, void *data);
        void makeWindowBlack();
        static void makeFullscreen(_GtkWidget *widget);
        static void makeUnfullscreen(_GtkWidget *widget);
        static void toggleFullscreen(_GtkWidget *widget);
        static void milhouseify(_GtkWidget *widget);

    private:

       GtkVideoSink(const GtkVideoSink&);     //No Copy Constructor
       GtkVideoSink& operator=(const GtkVideoSink&);     //No Assignment Operator
};


class XvImageSink
    : public GtkVideoSink, public BusMsgHandler
{
    public:
        XvImageSink(int screenNum) : GtkVideoSink(screenNum) {};
        bool handleBusMsg(_GstMessage *msg);

    private:
        void init();
        _GstElement *sinkElement() { return sink_; }
        ~XvImageSink();
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

    private:
        ~XImageSink();
        void init();
        // FIXME: need to implement this support in ximagesink
        void makeFullscreen() {}
        void makeUnfullscreen() {}

        _GstElement *sinkElement() { return colorspc_; }
        _GstElement *colorspc_;
        XImageSink(const XImageSink&);     //No Copy Constructor
        XImageSink& operator=(const XImageSink&);     //No Assignment Operator
};



#endif //_VIDEO_SINK_H_

