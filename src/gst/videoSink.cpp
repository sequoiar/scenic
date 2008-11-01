// videoSink.cpp
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

#include <cassert>

#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include "gstLinkable.h"
#include "videoSink.h"
#include "logWriter.h"
#include "pipeline.h"

//#define TRY_GL_VIDEOSINK

#ifdef TRY_GL_VIDEOSINK
#include "glVideoSink.cpp"
#else

void VideoSink::destroySink()
{
    stop();
    pipeline_.remove(&sink_);
}


gboolean XvImageSink::expose_cb(GtkWidget * widget, GdkEventExpose * /*event*/, gpointer data)
{
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(data), GDK_WINDOW_XWINDOW(widget->window));
    return TRUE;
}


gboolean XvImageSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer /*data*/)
{
    if (event->keyval != 'f')
    {
        LOG_DEBUG("user didn't hit f");
        return TRUE;
    }
    else
        LOG_DEBUG("you hit f");

    gboolean isFullscreen =
        (gdk_window_get_state(GDK_WINDOW(widget->window)) == GDK_WINDOW_STATE_FULLSCREEN);

    // toggle fullscreen state
    isFullscreen ? XvImageSink::makeUnfullscreen(widget) : XvImageSink::makeFullscreen(widget);

    return TRUE;
}


void XvImageSink::init()
{
    static bool gtk_initialized = false;
    if (!gtk_initialized)
        gtk_init(0, NULL);
    assert(sink_ = gst_element_factory_make("xvimagesink", "videosink"));
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);
    pipeline_.add(sink_);

    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window_);
    const gint WIDTH = 640;
    const gint HEIGHT = 480;

    gtk_window_set_default_size(GTK_WINDOW(window_), WIDTH, HEIGHT);
    gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

    g_signal_connect(G_OBJECT(window_), "expose-event", G_CALLBACK(
                         XvImageSink::expose_cb), static_cast<void*>(sink_));
    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
                     G_CALLBACK(XvImageSink::key_press_event_cb), NULL);
}


void XvImageSink::makeWindowBlack()
{
    GdkColor color;
    gdk_color_parse ("black", &color);
    gtk_widget_modify_bg(window_, GTK_STATE_NORMAL, &color);    // needed to ensure black background
}


void XvImageSink::showWindow()
{
    makeWindowBlack();
    gtk_window_set_title(GTK_WINDOW(window_), "Sropulpof");
    gtk_widget_show_all(window_);
}


void XvImageSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_fullscreen(GTK_WINDOW(widget));
}


void XvImageSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unfullscreen(GTK_WINDOW(widget));
}


XvImageSink::~XvImageSink()
{
    VideoSink::destroySink();
    if (window_)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Widget destroyed");
    }
}


void XImageSink::init()
{
    // ximagesink only supports rgb and not yuv colorspace, so we need a converter here
    assert(colorspc_ = gst_element_factory_make("ffmpegcolorspace", "colorspc"));
    pipeline_.add(colorspc_);

    assert(sink_ = gst_element_factory_make("ximagesink", "videosink"));
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);
    pipeline_.add(sink_);

    GstLinkable::link(colorspc_, sink_);
}


XImageSink::~XImageSink()
{
    VideoSink::destroySink();
    pipeline_.remove(&colorspc_);
}

GLfloat GLImageSink::x_ = -0.67f;
GLfloat GLImageSink::y_ = -0.5f;
GLfloat GLImageSink::z_ = -1.2f;
GLfloat GLImageSink::leftCrop_ = 0.0;
GLfloat GLImageSink::rightCrop_ = 0.0;
GLfloat GLImageSink::topCrop_ = 0.0;
GLfloat GLImageSink::bottomCrop_ = 0.0;

//client reshape callback
gboolean GLImageSink::reshapeCallback(GLuint width, GLuint height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (gfloat) width / (gfloat) height, 0.1, 100);  
    glMatrixMode(GL_MODELVIEW);	
    return TRUE;
}


//client draw callback
gboolean GLImageSink::drawCallback(GLuint texture, GLuint width, GLuint height)
{
    static GTimeVal current_time;
    static glong last_sec = current_time.tv_sec;
    static gint nbFrames = 0;  

    g_get_current_time (&current_time);
    nbFrames++ ;

    if ((current_time.tv_sec - last_sec) >= 1)
    {
        LOG_DEBUG("GRPHIC FPS = " << nbFrames << std::endl);

        nbFrames = 0;
        last_sec = current_time.tv_sec;
    }

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
    gfloat aspectRatio = (gfloat) width / (gfloat) height;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    
    glLoadIdentity();
    glColor3f(0.0f,0.0f,0.0f);
    glTranslatef(GLImageSink::x_, GLImageSink::y_, GLImageSink::z_);
    glTranslatef(GLImageSink::leftCrop_, GLImageSink::topCrop_,  0.01f);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(aspectRatio,  1.0f, 0.0f);
    glVertex3f(aspectRatio,  2.0f, 0.0f);
    glVertex3f(-aspectRatio, 2.0f, 0.0f);
    glVertex3f(-aspectRatio,0.0f,0.0f);
    glVertex3f(0.0f,0.0f,0.0f);
    glEnd();

    glLoadIdentity();
    glColor3f(0.0f,0.0f,0.0f);
    glTranslatef(GLImageSink::x_, GLImageSink::y_, GLImageSink::z_);
    glTranslatef(GLImageSink::rightCrop_, GLImageSink::bottomCrop_,  0.01f);
    glBegin(GL_TRIANGLE_FAN);
    
    glVertex3f(aspectRatio,0.0f,0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f,  -1.0f, 0.0f);
    glVertex3f(2*aspectRatio,  -1.0f, 0.0f); 
    glVertex3f(2*aspectRatio,  1.0f, 0.0f);
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

    glTranslatef(GLImageSink::x_, GLImageSink::y_, GLImageSink::z_);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);  glVertex3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f((gfloat)width-1, 0.0f);  glVertex3f(aspectRatio,  1.0f, 0.0f);
    glTexCoord2f((gfloat) width-1, (gfloat) height); glVertex3f(aspectRatio,  0.0f, 0.0f);
    glTexCoord2f(0.0f, height); glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd();

    //return TRUE causes a postRedisplay
    return TRUE;
}


gboolean GLImageSink::expose_cb(GtkWidget * widget, GdkEventExpose * /*event*/, gpointer data)
{
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(data), GDK_WINDOW_XWINDOW(widget->window));
    return TRUE;
}

const GLfloat step = 0.01;

gboolean GLImageSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer /*data*/)
{
    gboolean isFullscreen;
    switch (event->keyval) {
        case 'f':
            isFullscreen = (gdk_window_get_state(GDK_WINDOW(widget->window)) == GDK_WINDOW_STATE_FULLSCREEN);
            // toggle fullscreen state
            isFullscreen ? GLImageSink::makeUnfullscreen(widget) : GLImageSink::makeFullscreen(widget);
            break;
        case 'x':
        case GDK_Right:
            GLImageSink::x_ += step;
            break;
        case 'X':
        case GDK_Left:
            GLImageSink::x_ -= step;
            break;
        case 'y':
        case GDK_Down:
            GLImageSink::y_ += step;
            break;
        case 'Y':
        case GDK_Up:
            GLImageSink::y_ -= step;
            break;
        case 'z':
            GLImageSink::z_ += step;
            break;
        case 'Z':
            GLImageSink::z_ -= step;
            break;
        case 'b':
                GLImageSink::bottomCrop_ += step;
            break;
        case 'B':
                GLImageSink::bottomCrop_ -= step;
            break;
        case 't':
                GLImageSink::topCrop_ -= step;
            break;
        case 'T':
                GLImageSink::topCrop_ += step;
            break;
        case 'r':
                GLImageSink::rightCrop_ -= step;
            break;
        case 'R':
                GLImageSink::rightCrop_ += step;
            break;
        case 'l':
                GLImageSink::leftCrop_ += step;
            break;
        case 'L':
                GLImageSink::leftCrop_ -= step;
            break;
        default:
            g_print("unknown keypress %d", event->keyval);
            break;
    }
LOG_INFO("x:" << GLImageSink::x_  <<
" y:" << GLImageSink::y_ <<
" z:" << GLImageSink::z_ <<
" l:" << GLImageSink::leftCrop_ <<
" r:" << GLImageSink::rightCrop_ <<
" t:" <<  GLImageSink::topCrop_ <<
" b:" << GLImageSink::bottomCrop_);


    return TRUE;
}
void GLImageSink::makeWindowBlack()
{
    GdkColor color;
    gdk_color_parse ("black", &color);
    gtk_widget_modify_bg(window_, GTK_STATE_NORMAL, &color);    // needed to ensure black background
}


void GLImageSink::showWindow()
{
    makeWindowBlack();
    gtk_window_set_title(GTK_WINDOW(window_), "Sropulpof");
    gtk_widget_show_all(window_);
}


void GLImageSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_fullscreen(GTK_WINDOW(widget));
}


void GLImageSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unfullscreen(GTK_WINDOW(widget));
}


void GLImageSink::resetGLparams()
{
    // reset persistent static params to their initial values
    x_ = 0.0f;
    y_ = 0.0f;
    z_ = -5.0f;
    leftCrop_ = 0.0f;
    rightCrop_ = 0.0f;
    bottomCrop_ = 0.0f;
    topCrop_ = 0.0f;
}


GLImageSink::~GLImageSink()
{
    resetGLparams();

    pipeline_.remove(&glUpload_);
    VideoSink::destroySink();
    if (window_)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Widget destroyed");
    }
}


void GLImageSink::init()
{
    static bool gtk_initialized = false;
    if (!gtk_initialized)
        gtk_init(0, NULL);

    assert(glUpload_ = gst_element_factory_make("glupload", "colorspace"));
    pipeline_.add(glUpload_);

    assert(sink_ = gst_element_factory_make("glimagesink", "videosink"));
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    pipeline_.add(sink_);
    GstLinkable::link(glUpload_, sink_);

    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window_);
    const gint WIDTH = 640;
    const gint HEIGHT = 480;

    gtk_window_set_default_size(GTK_WINDOW(window_), WIDTH, HEIGHT);
    //gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

    g_signal_connect(G_OBJECT(window_), "expose-event", G_CALLBACK(
                GLImageSink::expose_cb), static_cast<void*>(sink_));
    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
            G_CALLBACK(GLImageSink::key_press_event_cb), NULL);

    /* configure elements */
    g_object_set(G_OBJECT(sink_), "client-reshape-callback", G_CALLBACK(GLImageSink::reshapeCallback), NULL);
    g_object_set(G_OBJECT(sink_), "client-draw-callback", G_CALLBACK(GLImageSink::drawCallback), NULL);  
}

#endif

