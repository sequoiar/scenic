// glVideoSink.h
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

#ifndef _GL_VIDEO_SINK_H_
#define _GL_VIDEO_SINK_H_

#include <GL/gl.h>
#include "gstLinkable.h"
#include "videoSink.h"
#include "busMsgHandler.h"

/** GLImageSink
 *  A videosink that can handle raw video, as well as GLBuffers of video.
 */
class Pipeline;

class GLImageSink
: public GtkVideoSink, private BusMsgHandler
{
    public:
        /// Constructor 
        GLImageSink(Pipeline &pipeline, int width, int height, int screen_num, unsigned long xid);
        bool handleBusMsg(_GstMessage *msg);

    private:

        /** 
         * Destructor */
        ~GLImageSink();

        _GstElement *sinkElement() { return sink_; }

        /** 
         * This method resets all of our static variables used for positioning our texture, 
         * and is called when this GLImageSink is destroyed. */
        static void resetGLparams();
        /** 
         * This method is invoked by the sink when it is reshaped */
        static int reshapeCallback(GLuint width, GLuint height);
        /** 
         * This method allows us to treat incoming buffers in an OpenGL context */
        static int drawCallback(GLuint texture, GLuint width, GLuint height);

        /** 
         * Mouse-wheel scroll event-handler which is used to move this GLImageSink's textures along the
         * Z-axis */
        static int mouse_wheel_cb(_GtkWidget *widget, _GdkEventScroll *event, void *data);
        /** 
         * Keypress event-handler which is used to map specific keys to parameters for translation and cropping 
         * applied to this GLImageSink's textures */
        static int key_press_event_cb(_GtkWidget *widget, _GdkEventKey *event,
                void *data);

        static const GLfloat STEP;
        static GLfloat x_;     
        static GLfloat y_;
        static GLfloat z_;
        static GLfloat leftCrop_;
        static GLfloat rightCrop_;
        static GLfloat bottomCrop_;
        static GLfloat topCrop_;

        static const GLfloat INIT_X;     
        static const GLfloat INIT_Y;
        static const GLfloat INIT_Z;
        static const GLfloat INIT_LEFT_CROP;
        static const GLfloat INIT_RIGHT_CROP;
        static const GLfloat INIT_BOTTOM_CROP;
        static const GLfloat INIT_TOP_CROP;
        static int window_width_;
        static int window_height_;
};

#endif //_GL_VIDEO_SINK_H_

