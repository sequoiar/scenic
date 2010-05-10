/* glVideoSink.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"
#include "videoSize.h"

#ifdef CONFIG_GL

#include <gst/interfaces/xoverlay.h>
#include "gstLinkable.h"
#include "pipeline.h"
#include "playback.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <X11/extensions/Xinerama.h>

#include <GL/glu.h>

#include "glVideoSink.h"

        
const GLfloat NTSC_VIDEO_RATIO = 4.0 / 3.0;
const GLfloat GLImageSink::INIT_X = -0.5 * NTSC_VIDEO_RATIO; 
const GLfloat GLImageSink::INIT_Y = -0.5;
const GLfloat GLImageSink::INIT_Z = -1.2175;

const GLfloat GLImageSink::INIT_LEFT_CROP = 0.0;
const GLfloat GLImageSink::INIT_RIGHT_CROP = 0.0;
const GLfloat GLImageSink::INIT_BOTTOM_CROP = 0.0;
const GLfloat GLImageSink::INIT_TOP_CROP = 0.0;
const GLfloat GLImageSink::STEP = 0.1;

GLfloat GLImageSink::x_ = INIT_X;
GLfloat GLImageSink::y_ = INIT_Y;
GLfloat GLImageSink::z_ = INIT_Z;
GLfloat GLImageSink::leftCrop_ = INIT_LEFT_CROP;
GLfloat GLImageSink::rightCrop_ = INIT_RIGHT_CROP;
GLfloat GLImageSink::topCrop_ = INIT_TOP_CROP;
GLfloat GLImageSink::bottomCrop_ = INIT_BOTTOM_CROP;

bool GLImageSink::handleBusMsg(GstMessage * message)
{
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return false;
 
    if (!gst_structure_has_name(message->structure, "prepare-xwindow-id"))
        return false;
 
    LOG_DEBUG("Got prepare-xwindow-id msg");
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), getXWindow());
  
    return true;
}

GLImageSink::GLImageSink(Pipeline &pipeline, int width, int height, unsigned long xid) : 
    GtkVideoSink(pipeline, xid), 
    BusMsgHandler(&pipeline)
{
    sink_ = VideoSink::pipeline_.makeElement("glimagesink", NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);

    if (hasWindow())
    {
        gtk_window_set_default_size(GTK_WINDOW(window_), width, height);
        //gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title
        g_signal_connect(G_OBJECT(window_), "key-press-event",
                G_CALLBACK(key_press_event_cb), this);
        g_signal_connect(G_OBJECT(window_), "scroll-event",
                G_CALLBACK(mouse_wheel_cb), NULL);
        g_signal_connect(G_OBJECT(window_), "destroy",
                G_CALLBACK(destroy_cb), static_cast<gpointer>(this));

        gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
        gtk_widget_set_events(window_, GDK_SCROLL_MASK);

        /* configure elements */
        //g_object_set(G_OBJECT(sink_), "client-reshape-callback", G_CALLBACK(reshapeCallback), NULL);
        //g_object_set(G_OBJECT(sink_), "client-draw-callback", G_CALLBACK(drawCallback), NULL);  
        showWindow();
        hideCursor();
        gtk_widget_set_size_request(drawingArea_, width, height);
    }
}

gboolean GLImageSink::reshapeCallback(GLuint width, GLuint height)
{
    const static GLfloat vwinRatio = (gfloat) videosize::WIDTH / (gfloat) videosize::HEIGHT ;
    LOG_DEBUG("WIDTH: " << width << ", HEIGHT: " << height << std::endl);

    // /TODO:oldDOCS
    // explain below -- ( screen x - ( needed x res)) == extra space
    //move origin to extra space / 2 -- so that quad is in the middle of screen
    //: Why  the disparity between 4/3 and videosink aspect?   
    if (width > height)
        glViewport((width - height * vwinRatio) * 0.5, 0, height * vwinRatio, height);
    else
        glViewport(0, (height - (width * (1.0 / vwinRatio))) * 0.5, width, (float) width * (1.0 / vwinRatio));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45, NTSC_VIDEO_RATIO , 0.1, 100);  

    glMatrixMode(GL_MODELVIEW);	
    return TRUE;
}


gboolean GLImageSink::drawCallback(GLuint texture, GLuint width, GLuint height)
{
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);

    gfloat aspectRatio = (gfloat) width / (gfloat) height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(1.0f,0.0f,0.0f);
    glTranslatef(x_, y_, z_);
    glTranslatef(leftCrop_, topCrop_,  0.01f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(aspectRatio,  1.0f, 0.0f);
    glVertex3f(aspectRatio,  2.0f, 0.0f);
    glVertex3f(-aspectRatio, 2.0f, 0.0f);
    glVertex3f(-aspectRatio,0.0f,0.0f);
    glVertex3f(0.0f,0.0f,0.0f);
    glEnd();

    glLoadIdentity();
    glColor3f(0.0f,1.0f,0.0f);
    glTranslatef(x_, y_, z_);
    glTranslatef(rightCrop_, bottomCrop_,  0.01f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(aspectRatio,0.0f,0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f,  -1.0f, 0.0f);
    glVertex3f(2 * aspectRatio,  -1.0f, 0.0f); 
    glVertex3f(2 * aspectRatio,  1.0f, 0.0f);
    glVertex3f(aspectRatio,  1.0f, 0.0f);
    glEnd();

    glEnable (GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, texture);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glLoadIdentity();

    glTranslatef(x_, y_, z_);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);  glVertex3f(0.0f, 1.0, 0.0f);
    glTexCoord2f((gfloat) width - 1, 0.0f);  glVertex3f(aspectRatio, 1.0f, 0.0f);
    glTexCoord2f((gfloat) width - 1, (gfloat) height); glVertex3f(aspectRatio, 0.0f, 0.0f);
    glTexCoord2f(0.0f, height); glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd();

    //return TRUE causes a postRedisplay
    return FALSE;
}


gboolean GLImageSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    GLImageSink *context = static_cast<GLImageSink*>(data);
    switch (event->keyval) {
        case GDK_f:
        case GDK_F:
        case GDK_Escape:
            context->toggleFullscreen(widget);
            break;
        case GDK_x:
        case GDK_Right:
            x_ += STEP;
            break;
        case GDK_X:
        case GDK_Left:
            x_ -= STEP;
            break;
        case GDK_y:
        case GDK_Down:
            y_ += STEP;
            break;
        case GDK_Y:
        case GDK_Up:
            y_ -= STEP;
            break;
        case GDK_z:
            z_ += STEP;
            break;
        case GDK_Z:
            z_ -= STEP;
            break;
        case GDK_b:
            bottomCrop_ += STEP;
            break;
        case GDK_B:
            bottomCrop_ -= STEP;
            break;
        case GDK_t:
            topCrop_ -= STEP;
            break;
        case GDK_T:
            topCrop_ += STEP;
            break;
        case GDK_r:
            rightCrop_ -= STEP;
            break;
        case GDK_R:
            rightCrop_ += STEP;
            break;
        case GDK_l:
            leftCrop_ += STEP;
            break;
        case GDK_L:
            leftCrop_ -= STEP;
            break;
        case GDK_c:
        case GDK_C:
            LOG_DEBUG("Resetting GL texture position");
            resetGLparams();
            break;
        case GDK_q:
            // Quit application on ctrl-q, this quits the main loop
            // (if there is one)
            if (event->state & GDK_CONTROL_MASK)
            {
                LOG_INFO("Ctrl-Q key pressed, quitting.");
                context->VideoSink::pipeline_.quit();
            }
            break;
        default:
            LOG_DEBUG("unknown keypress " << event->keyval);
            break;
    }
#if LOG_COORD_VARS
    LOG_INFO("x:" << x_  <<
            " y:" << y_ <<
            " z:" << z_ <<
            " l:" << leftCrop_ <<
            " r:" << rightCrop_ <<
            " t:" <<  topCrop_ <<
            " b:" << bottomCrop_);
#endif

    return TRUE;
}


gboolean GLImageSink::mouse_wheel_cb(GtkWidget * /*widget*/, GdkEventScroll *event, gpointer /*data*/)
{
    switch (event->direction)
    {
        case GDK_SCROLL_UP:
            z_ += STEP;
            break;
        case GDK_SCROLL_DOWN:
            z_ -= STEP;
            break;
        default:
            LOG_DEBUG("Unhandled mouse wheel event");
            break;
    }
    return TRUE;
}


void GLImageSink::resetGLparams()
{
    // reset persistent static params to their initial values
    x_ = INIT_X;
    y_ = INIT_Y;
    z_ = INIT_Z; 
    leftCrop_ = INIT_LEFT_CROP;
    rightCrop_ = INIT_RIGHT_CROP;
    bottomCrop_ = INIT_BOTTOM_CROP;
    topCrop_ = INIT_TOP_CROP;
}


GLImageSink::~GLImageSink()
{
    resetGLparams();

    GtkVideoSink::destroySink();
    if (window_)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Widget destroyed");
    }
}

#endif //CONFIG_GL

