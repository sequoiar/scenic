// videoSink.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _VIDEO_SINK_H_
#define _VIDEO_SINK_H_

#include <string>
#include <gdk/gdkevents.h>

#include "noncopyable.h"

class Pipeline;
class _GtkWidget;
class _GdkEventKey;
class _GdkEventScroll;
class _GstElement;
class _GdkEventWindowState;

class VideoSink : private boost::noncopyable
{
    public:
        VideoSink() : sink_(0) {};
        virtual ~VideoSink(){};
        virtual _GstElement* sinkElement() { return sink_; }

    protected:
        _GstElement *sink_;
};

class XvImageSink
: public VideoSink
{
    public:
        XvImageSink(Pipeline &pipeline, int width, int height, 
                unsigned long xid, const std::string &display, const std::string &title);
        void toggleFullscreen();

    private:
        void updateDisplay(const std::string &display);

        unsigned long xid_;
        bool isFullscreen_;

        _GtkWidget *window_;
        _GtkWidget *drawingArea_;
        _GtkWidget *vbox_;
        _GtkWidget *hbox_;
        _GtkWidget *horizontalSlider_;
        _GtkWidget *sliderFrame_;

        static int onWindowStateEvent(_GtkWidget *widget, _GdkEventWindowState *event, void *data);
        static void window_closed(_GtkWidget * widget, _GdkEvent *event, void *data);
        void makeFullscreen(_GtkWidget *widget);
        void makeUnfullscreen(_GtkWidget *widget);
        void toggleFullscreen(_GtkWidget *widget);
        void hideCursor();
        void showCursor();
        bool hasWindow() const;

        virtual ~XvImageSink();
        static int key_press_event_cb(_GtkWidget *widget, _GdkEventKey *event,
                void *data);
};


class XImageSink
: public VideoSink
{
    public: 
        XImageSink(const Pipeline &pipeline, const std::string &display);

    private:
        virtual _GstElement *sinkElement();
        _GstElement *colorspace_;
};

#endif //_VIDEO_SINK_H_

