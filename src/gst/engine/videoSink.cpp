/* videoSink.cpp
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

#include <gst/interfaces/xoverlay.h>
#include "gstLinkable.h"
#include "videoSink.h"
#include "pipeline.h"
#include "playback.h"
#include "gutil.h"


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <X11/extensions/Xinerama.h>

const unsigned int GtkVideoSink::WIDTH = 720;
const unsigned int GtkVideoSink::HEIGHT = 528;

void VideoSink::destroySink()
{
    Pipeline::Instance()->remove(&sink_);
}

Window GtkVideoSink::getXWindow()
{ 
    return GDK_WINDOW_XWINDOW(window_->window);
}


gboolean GtkVideoSink::expose_cb(GtkWidget * widget, GdkEventExpose * /*event*/, gpointer data)
{
    GtkVideoSink *context = static_cast<GtkVideoSink*>(data);
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(context->sink_), GDK_WINDOW_XWINDOW(widget->window));
    return TRUE;
}


void GtkVideoSink::makeWindowBlack()
{
    GdkColor color;
    gdk_color_parse ("black", &color);
    gtk_widget_modify_bg(window_, GTK_STATE_NORMAL, &color);    // needed to ensure black background
}


void GtkVideoSink::showWindow()
{
    makeWindowBlack();
    gtk_window_set_title(GTK_WINDOW(window_), "Milhouse");
    gtk_widget_show_all(window_);
}


void GtkVideoSink::toggleFullscreen(GtkWidget *widget)
{
#if 0
    gboolean isFullscreen =
        (gdk_window_get_state(GDK_WINDOW(widget->window)) == GDK_WINDOW_STATE_FULLSCREEN);
#endif
    // FIXME: this could be flipped if the window manager changes the fullscreen state
    static gboolean isFullscreen = FALSE;

    // toggle fullscreen state
    isFullscreen ? makeUnfullscreen(widget) : makeFullscreen(widget);
    isFullscreen = !isFullscreen;
}


void GtkVideoSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget));           // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
}


void GtkVideoSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget));           // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
}


void GtkVideoSink::prepareSink()
{
    //g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);
}


gboolean XvImageSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer /*data*/)
{
    switch (event->keyval)
    {
        case GDK_f:
        case GDK_F:
            toggleFullscreen(widget);
            break;

        case GDK_Q:
            // Quit application, this quits the main loop
            // (if there is one)
                LOG_INFO("Q key pressed, quitting.");
                playback::quit();
            break;

        default:
            break;
    }

    return TRUE;
}


void XvImageSink::init()
{
    static bool gtk_initialized = false;
    if (!gtk_initialized)
    {
        gtk_init(0, NULL);
        gtk_initialized = true;
    }

    sink_ = Pipeline::Instance()->makeElement("xvimagesink", "videosink");
    prepareSink();

    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    assert(window_);

    GdkDisplay* display = gdk_display_get_default();
    assert(display);
    int n;
    XineramaScreenInfo* xine = XineramaQueryScreens(GDK_DISPLAY_XDISPLAY(display), &n);
    if (!xine)
        n = 0; // don't query ScreenInfo
    for (int j = 0; j < n; ++j)
    {
        LOG_INFO(   "req:" << screen_num_ << 
                " screen:" << xine[j].screen_number << 
                " x:" << xine[j].x_org << 
                " y:" << xine[j].y_org << 
                " width:" << xine[j].width << 
                " height:" << xine[j].height);
        if (j == screen_num_) 
            gtk_window_move(GTK_WINDOW(window_), xine[j].x_org,xine[j].y_org);
    }

    gtk_window_set_default_size(GTK_WINDOW(window_), GtkVideoSink::WIDTH, GtkVideoSink::HEIGHT);
    //gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

    //g_signal_connect(G_OBJECT(window_), "destroy", G_CALLBACK(gutil::killMainLoop), NULL);

    g_signal_connect(G_OBJECT(window_), "expose-event", G_CALLBACK(
                expose_cb), static_cast<void*>(this));
    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
            G_CALLBACK(XvImageSink::key_press_event_cb), NULL);

    showWindow();
}


XvImageSink::~XvImageSink()
{
    GtkVideoSink::destroySink();
    if (window_)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Widget destroyed");
    }
}


void XImageSink::init()
{
    // ximagesink only supports rgb and not yuv colorspace, so we need a converter here
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", "colorspc");

    sink_ = Pipeline::Instance()->makeElement("ximagesink", "videosink");
    g_object_set(sink_, "pixel-aspect-ratio", "10/11", NULL);
    g_object_set(sink_, "force-aspect-ratio", TRUE, NULL);
    //    prepareSink();

    gstlinkable::link(colorspc_, sink_);
}


XImageSink::~XImageSink()
{
    VideoSink::destroySink();
    Pipeline::Instance()->remove(&colorspc_);
}

