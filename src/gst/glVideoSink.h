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

#ifndef _GL_VIDEO_SINK_H_
#define _GL_VIDEO_SINK_H_

#include "gstLinkable.h"
#include <GL/gl.h>
#include "videoSink.h"
#include "busMsgHandler.h"

class GLImageSink
: public VideoSink, public BusMsgHandler
{
    public:
        GLImageSink() : glUpload_(0){};
        ~GLImageSink();
        void init();
        _GstElement *sinkElement() { return glUpload_; }
    private:
        static void resetGLparams();
        static int reshapeCallback(GLuint width, GLuint height);
        static int drawCallback(GLuint texture, GLuint width, GLuint height);

        static int mouse_wheel_cb(_GtkWidget *widget, _GdkEventScroll *event, void *data);
        static int key_press_event_cb(_GtkWidget *widget, _GdkEventKey *event,
                void *data);

        bool handleBusMsg(_GstMessage *msg);

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

#endif //_GL_VIDEO_SINK_H_

