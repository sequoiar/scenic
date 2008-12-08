/* videoSink.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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
#include <cassert>

#include <gst/interfaces/xoverlay.h>
#include "gstLinkable.h"
#include "videoSink.h"
#include "logWriter.h"
#include "pipeline.h"
#include "playback.h"

#include "logWriter.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <X11/extensions/Xinerama.h>

const unsigned int VideoSink::WIDTH = 720;
const unsigned int VideoSink::HEIGHT = 528;

void VideoSink::destroySink()
{
    Pipeline::Instance()->remove(&sink_);
}

Window VideoSink::getXWindow()
{ 
    return GDK_WINDOW_XWINDOW(window_->window);
}


gboolean VideoSink::expose_cb(GtkWidget * widget, GdkEventExpose * /*event*/, gpointer data)
{
    VideoSink *context = static_cast<VideoSink*>(data);
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(context->sink_), GDK_WINDOW_XWINDOW(widget->window));
    return TRUE;
}


void VideoSink::makeWindowBlack()
{
    GdkColor color;
    gdk_color_parse ("black", &color);
    gtk_widget_modify_bg(window_, GTK_STATE_NORMAL, &color);    // needed to ensure black background
}


void VideoSink::showWindow()
{
    makeWindowBlack();
    gtk_window_set_title(GTK_WINDOW(window_), "Sropulpof");
    gtk_widget_show_all(window_);
}


void VideoSink::toggleFullscreen(GtkWidget *widget)
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


void VideoSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget));           // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
}


void VideoSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget));           // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
}


void VideoSink::prepareSink()
{
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);
}


gboolean XvImageSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer /*data*/)
{
    switch (event->keyval)
    {
        case 'f':
            toggleFullscreen(widget);
            break;

        case 'q':
        case 'Q':
        case GDK_Escape: // escape character
            // Quit application, this quits the main loop
            // (if there is one)
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

    gtk_window_set_default_size(GTK_WINDOW(window_), VideoSink::WIDTH, VideoSink::HEIGHT);
    gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

    g_signal_connect(G_OBJECT(window_), "expose-event", G_CALLBACK(
                expose_cb), static_cast<void*>(this));
    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
            G_CALLBACK(XvImageSink::key_press_event_cb), NULL);
    showWindow();
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
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", "colorspc");

    sink_ = Pipeline::Instance()->makeElement("ximagesink", "videosink");
    prepareSink();

    gstlinkable::link(colorspc_, sink_);
}


XImageSink::~XImageSink()
{
    VideoSink::destroySink();
    Pipeline::Instance()->remove(&colorspc_);
}

