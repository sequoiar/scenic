// videoSink.cpp
// Copyright 2008 Koya Charles & Tristan Matthews
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
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
#include <gdk/gdkx.h>

#include "gstLinkable.h"
#include "videoSink.h"
#include "logWriter.h"
#include "pipeline.h"


void VideoSink::destroySink()
{
    stop();
    pipeline_.remove(&sink_);
}

Window         window = 0;

gboolean XvImageSink::expose_cb(GtkWidget * widget, GdkEventExpose * /*event*/, gpointer data)
{

    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(data), GDK_WINDOW_XWINDOW(widget->window));
    ::window =GDK_WINDOW_XWINDOW(widget->window);
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
#include<GL/gl.h>
#include<GL/glx.h>
//#include<GL/glu.h>

Display                 *dpy;
Window                  root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;
Pixmap			pixmap;
int			pixmap_width = 200, pixmap_height = 200;
GC			gc;
XImage			*xim;
GLuint			texture_id;
GLfloat     r_angle = 0.0;   
bool        glDone = false;
Pixmap pix()
{


 dpy = XOpenDisplay(NULL);
 
 if(dpy == NULL) {
        exit(0); }
        
 root = DefaultRootWindow(dpy);
 
 vi = glXChooseVisual(dpy, 0, att);

 if(vi == NULL) {
        exit(0); }
        
 swa.event_mask = ExposureMask | KeyPressMask;
 swa.colormap   = XCreateColormap(dpy, root, vi->visual, AllocNone);

 win = XCreateWindow(dpy, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWEventMask  | CWColormap, &swa);
 XMapWindow(dpy, win);
 XStoreName(dpy, win, "PIXMAP TO TEXTURE");

 glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
 if(glc == NULL) {
	exit(0); }

 glXMakeCurrent(dpy, win, glc);
 glEnable(GL_DEPTH_TEST);
 

 glEnable(GL_TEXTURE_2D);
 glGenTextures(1, &texture_id);
 glBindTexture(GL_TEXTURE_2D, texture_id);
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


 return pixmap;
}
gboolean Redraw(gpointer) {
 XWindowAttributes	gwa;
 if(glDone)
     return FALSE;
 if(!window)
     return TRUE;


// xim = XGetImage(dpy, window, 0, 0, pixmap_width, pixmap_height, AllPlanes, ZPixmap);
 /* CREATE A PIXMAP AND DRAW SOMETHING */

 pixmap	= XCreatePixmap(dpy, root, pixmap_width, pixmap_height, vi->depth);
 gc = DefaultGC(dpy, 0);

 XSetForeground(dpy, gc, 0x00c0c0);
 XFillRectangle(dpy, pixmap, gc, 0, 0, pixmap_width, pixmap_height);

 XSetForeground(dpy, gc, 0x000000);
 XFillArc(dpy, pixmap, gc, 15, 25, 50, 50, 0, 360*64);

 XSetForeground(dpy, gc, 0x0000ff);
 XDrawString(dpy, pixmap, gc, 10, 15, "PIXMAP TO TEXTURE", strlen("PIXMAP TO TEXTURE"));

 XSetForeground(dpy, gc, 0xff0000);
 XFillRectangle(dpy, pixmap, gc, 75, 75, 45, 35);

 XFlush(dpy);
 xim = XGetImage(dpy, pixmap, 0, 0, pixmap_width, pixmap_height, AllPlanes, ZPixmap);

 if(xim == NULL) {
 	printf("\n\tximage could not be created.\n\n"); }

LOG_DEBUG("Byte order:" << xim->byte_order);
        glPixelStorei (GL_UNPACK_SWAP_BYTES, GL_FALSE);
        glPixelStorei (GL_UNPACK_LSB_FIRST, GL_FALSE);
        glPixelStorei (GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei (GL_UNPACK_SKIP_ROWS, 0);
        glPixelStorei (GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei (GL_UNPACK_ALIGNMENT, 1);


 glTexImage2D(GL_TEXTURE_2D, 0, BYTES_PP, pixmap_height, pixmap_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)(&(xim->data[0])));

 XGetWindowAttributes(dpy, win, &gwa);
 glViewport(0, 0, gwa.width, gwa.height);
 glClearColor(0.3, 0.3, 0.3, 1.0);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
// glOrtho(-1.25, 1.25, -1.25, 1.25, 1., 20.);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glTranslatef(0.0f,0.0f,-1.0f);  
//gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

 glColor3f(1.0, 1.0, 1.0);
 r_angle+=2;
 r_angle = r_angle > 360.0? r_angle-360.0: r_angle;
 glRotatef(r_angle,0.0f,1.0f,1.0f);    

 glBegin(GL_QUADS);
        /* front face */
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f); 
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        /* back face */
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f); 
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        /* right face */
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f); 
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        /* left face */
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f); 
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        /* top face */
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 1.0f); 
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        /* bottom face */
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f); 
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);

 glEnd(); 

 glXSwapBuffers(dpy, win); 
 
 return TRUE;
}

void XvImageSink::init()
{
    static bool gtk_initialized = false;
    if (!gtk_initialized)
        gtk_init(0, NULL);
pix();
    assert(sink_ = gst_element_factory_make("xvimagesink", "videosink"));
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);
    pipeline_.add(sink_);

    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window_);
    g_signal_connect(G_OBJECT(window_), "expose-event", G_CALLBACK(
                         XvImageSink::expose_cb), static_cast<void*>(sink_));
    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
                     G_CALLBACK(XvImageSink::key_press_event_cb), NULL);
    g_timeout_add(20,Redraw,NULL);
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
    glDone = true;
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

