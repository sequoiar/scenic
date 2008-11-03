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
#include <GL/glew.h>

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
            : sink_(0) {};
        ~VideoSink(){};
        virtual void showWindow() {};   // FIXME: not useful for ximagesink
        virtual void makeFullscreen() = 0;
        virtual void makeUnfullscreen() = 0;

        _GstElement *sinkElement() { return sink_; }
        
    protected:
        _GstElement *sink_;
        void destroySink();

    private:
        VideoSink(const VideoSink&);     //No Copy Constructor
        VideoSink& operator=(const VideoSink&);     //No Assignment Operator
};


class XvImageSink
    : public VideoSink
{
    public:
        XvImageSink()
            : window_(0) {};
        ~XvImageSink();
        void init();
        void showWindow();
        void makeFullscreen() { makeFullscreen(window_); }
        void makeUnfullscreen() { makeUnfullscreen(window_); }

    private:
        static void makeFullscreen(_GtkWidget *widget);
        static void makeUnfullscreen(_GtkWidget *widget);

        static int key_press_event_cb(_GtkWidget *widget, _GdkEventKey *event,
                void *data);
        static int expose_cb(_GtkWidget *widget, _GdkEventExpose *event, void *data);
        void makeWindowBlack();

        _GtkWidget *window_;
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


class GLImageSink
: public VideoSink
{
    public:
        GLImageSink() : window_(0), glUpload_(0){};
        ~GLImageSink();
        void init();
        void showWindow();
        void makeFullscreen() { makeFullscreen(window_); }
        void makeUnfullscreen() { makeUnfullscreen(window_); }
        _GstElement *sinkElement() { return glUpload_; }
    private:
        static void resetGLparams();
        static int reshapeCallback(GLuint width, GLuint height);
        static int drawCallback(GLuint texture, GLuint width, GLuint height);
        static void makeFullscreen(_GtkWidget *widget);
        static void makeUnfullscreen(_GtkWidget *widget);

        static int mouse_wheel_cb(_GtkWidget *widget, _GdkEventScroll *event, void *data);
        static int key_press_event_cb(_GtkWidget *widget, _GdkEventKey *event,
                void *data);
        static int expose_cb(_GtkWidget *widget, _GdkEventExpose *event, void *data);
        void makeWindowBlack();

        _GtkWidget *window_;
        _GstElement *glUpload_;
        static const GLfloat STEP;
        static GLfloat x_;     // FIXME: separate out gl stuff into separate class
        static GLfloat y_;
        static GLfloat z_;
        static GLfloat leftCrop_;
        static GLfloat rightCrop_;
        static GLfloat bottomCrop_;
        static GLfloat topCrop_;

        GLImageSink(const GLImageSink&);     //No Copy Constructor
        GLImageSink& operator=(const GLImageSink&);     //No Assignment Operator
};

#endif //_VIDEO_SINK_H_

