// videoSource.cpp
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

#include <iostream>
#include <string>
#include <cassert>

#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "gstLinkable.h"
#include "videoSink.h"
#include "logWriter.h"


gboolean VideoSink::expose_cb(GtkWidget * widget, GdkEventExpose * /*event*/, gpointer data)
{
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(data), GDK_WINDOW_XWINDOW(widget->window));
    gtk_widget_show_all(widget);
    return TRUE;
}


gboolean VideoSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer /*data*/)
{
    if (event->keyval != 'f')
    {
        g_print("you didn't hit f, jerk\n");
        return TRUE;
    }
    else 
        g_print("you hit f\n");

    gboolean isFullscreen = (gdk_window_get_state(GDK_WINDOW(widget->window)) == GDK_WINDOW_STATE_FULLSCREEN);

    if (isFullscreen) 
        gtk_window_unfullscreen(GTK_WINDOW(widget));
    else
        gtk_window_fullscreen(GTK_WINDOW(widget));

    return TRUE;
}


// parts of sub_init that are common to all VideoSource classes
void VideoSink::init()
{
    static bool gtk_initialized = false;
    if (!gtk_initialized)
        gtk_init(0, NULL);

    assert(sink_ = gst_element_factory_make("xvimagesink", "videosink"));
    pipeline_.add(sink_);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);
    
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window_), "expose-event", G_CALLBACK(VideoSink::expose_cb), static_cast<void*>(sink_));
    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event", G_CALLBACK(VideoSink::key_press_event_cb), NULL);
}


void VideoSink::showWindow()
{
    gtk_widget_show_all(window_);
}


VideoSink::~VideoSink()
{
    assert(stop());
    pipeline_.remove(sink_);
    if (window_)
    {
        LOG("Widget destroyed", DEBUG);
        gtk_widget_destroy(window_);
    }
}


