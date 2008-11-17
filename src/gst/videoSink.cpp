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

#include <gst/interfaces/xoverlay.h>
#include "gstLinkable.h"
#include "videoSink.h"
#include "logWriter.h"
#include "pipeline.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

void VideoSink::destroySink()
{
    stop();
    pipeline_.remove(&sink_);
}


gboolean VideoSink::expose_cb(GtkWidget * widget, GdkEventExpose * /*event*/, gpointer data)
{
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(data), GDK_WINDOW_XWINDOW(widget->window));
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
    gboolean isFullscreen =
        (gdk_window_get_state(GDK_WINDOW(widget->window)) == GDK_WINDOW_STATE_FULLSCREEN);

    // toggle fullscreen state
    isFullscreen ? makeUnfullscreen(widget) : makeFullscreen(widget);
}


void VideoSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_fullscreen(GTK_WINDOW(widget));
}


void VideoSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unfullscreen(GTK_WINDOW(widget));
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

    toggleFullscreen(widget);
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

    gstlinkable::link(colorspc_, sink_);
}


XImageSink::~XImageSink()
{
    VideoSink::destroySink();
    pipeline_.remove(&colorspc_);
}


