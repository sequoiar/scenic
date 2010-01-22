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

#include <string>
#include <X11/Xlib.h>
#include "gstLinkable.h"
#include "busMsgHandler.h"
#include "messageHandler.h"

#include "noncopyable.h"

class Pipeline;
class _GtkWidget;
class _GdkEventExpose;
class _GdkEventKey;
class _GdkEventScroll;
class _GstElement;

class VideoSink : public GstLinkableSink, boost::noncopyable
{
    public:
        explicit VideoSink(Pipeline &pipeline) : pipeline_(pipeline), sink_(0) {};
        virtual ~VideoSink() {};

    protected:
        virtual void destroySink();
        void prepareSink();
        Pipeline &pipeline_;
        _GstElement *sink_;
};

class GtkVideoSink
: public VideoSink, public MessageHandler
{
    public:
        GtkVideoSink(Pipeline &pipeline, int screen_num)
            : VideoSink(pipeline), window_(0), screen_num_(screen_num) {};
        virtual ~GtkVideoSink(){};
        void showWindow();


    protected:
        void toggleFullscreen() { toggleFullscreen(window_); }
        _GtkWidget *window_;
        int screen_num_;

        static void destroy_cb(_GtkWidget * /*widget*/, void *data);
        Window getXWindow();
        static int expose_cb(_GtkWidget *widget, _GdkEventExpose *event, void *data);
        void makeWindowBlack();
        static void makeFullscreen(_GtkWidget *widget);
        static void makeUnfullscreen(_GtkWidget *widget);
        static void toggleFullscreen(_GtkWidget *widget);
        void hideCursor();

    private:
        virtual bool handleMessage(const std::string &path, const std::string &arguments);
};


class XvImageSink
: public GtkVideoSink, public BusMsgHandler
{
    public:
        XvImageSink(Pipeline &pipeline, int width, int height, int screenNum);
        bool handleBusMsg(_GstMessage *msg);

    private:
        _GstElement *sinkElement() { return sink_; }
        ~XvImageSink();
        static int key_press_event_cb(_GtkWidget *widget, _GdkEventKey *event,
                void *data);

};


class XImageSink
: public VideoSink
{
    public: 
        XImageSink(Pipeline &pipeline); 

    private:
        ~XImageSink();

        _GstElement *sinkElement() { return colorspc_; }
        _GstElement *colorspc_;
};



#endif //_VIDEO_SINK_H_

