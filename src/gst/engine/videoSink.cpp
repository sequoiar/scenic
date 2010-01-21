/* videoSink.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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


void VideoSink::destroySink()
{
    Pipeline::Instance()->remove(&sink_);
}

Window GtkVideoSink::getXWindow()
{ 
    // FIXME: see https://bugzilla.gnome.org/show_bug.cgi?id=599885
    return GDK_WINDOW_XWINDOW(window_->window);
}


void GtkVideoSink::destroy_cb(GtkWidget * /*widget*/, gpointer data)
{
    LOG_DEBUG("Window closed, quitting.");
    playback::quit();
    GtkVideoSink *context = static_cast<GtkVideoSink*>(data);
    context->window_ = 0;
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
    gtk_widget_show_all(window_);
}


void GtkVideoSink::hideCursor()
{
    // FIXME: this is because gtk doesn't support GDK_BLANK_CURSOR before gtk-2.16
    char invisible_cursor_bits[] = { 0x0 };
    GdkCursor* cursor;
    GdkBitmap *empty_bitmap;
    GdkColor color = { 0, 0, 0, 0 };
    empty_bitmap = gdk_bitmap_create_from_data (GDK_WINDOW(window_->window),
            invisible_cursor_bits,
            1, 1);

    cursor = gdk_cursor_new_from_pixmap(empty_bitmap, empty_bitmap, &color,
            &color, 0, 0);

    gdk_window_set_cursor(GDK_WINDOW(window_->window), cursor);
}


void GtkVideoSink::toggleFullscreen(GtkWidget *widget)
{
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


bool GtkVideoSink::handleMessage(const std::string &path, const std::string &arguments)
{
    if (path == "fullscreen")
    {
        toggleFullscreen();
        return true;
    }
    else if (path == "window-title")
    {
        gtk_window_set_title(GTK_WINDOW(window_), arguments.c_str());
        return true;
    }

    return false;
}


void VideoSink::prepareSink()
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


bool XvImageSink::handleBusMsg(GstMessage * message)
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

XvImageSink::XvImageSink(int width, int height, int screenNum) : 
    GtkVideoSink(screenNum) 
{
    static bool gtk_initialized = false;
    if (!gtk_initialized)
    {
        gtk_init(0, NULL);
        gtk_initialized = true;
    }

    sink_ = Pipeline::Instance()->makeElement("xvimagesink", NULL);
    prepareSink();

    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    tassert(window_);

    /// FIXME: this is ifdef'd out to avoid getting that  Xinerama error msg every time
#ifdef XINE_QUERY
    GdkDisplay* display = gdk_display_get_default();
    tassert(display);
    int n;
    XineramaScreenInfo* xine = XineramaQueryScreens(GDK_DISPLAY_XDISPLAY(display), &n);
    if (!xine)
        n = 0; // don't query ScreenInfo
    for (int j = 0; j < n; ++j)
    {
        LOG_INFO(" req:"    << screen_num_ << 
                 " screen:" << xine[j].screen_number << 
                 " x:"      << xine[j].x_org << 
                 " y:"      << xine[j].y_org << 
                 " width:"  << xine[j].width << 
                 " height:" << xine[j].height);
        if (j == screen_num_) 
            gtk_window_move(GTK_WINDOW(window_), xine[j].x_org, xine[j].y_org);
    }
#endif

    LOG_DEBUG("Setting default window size to " << width << "x" << height);
    gtk_window_set_default_size(GTK_WINDOW(window_), width, height);
    //gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
            G_CALLBACK(XvImageSink::key_press_event_cb), NULL);
    g_signal_connect(G_OBJECT(window_), "destroy",
            G_CALLBACK(destroy_cb), static_cast<gpointer>(this));

    showWindow();
    hideCursor();
}


XvImageSink::~XvImageSink()
{
    GtkVideoSink::destroySink();
    if (window_)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Videosink window destroyed");
    }
}


XImageSink::XImageSink() : 
    colorspc_(Pipeline::Instance()->makeElement("ffmpegcolorspace", NULL)) 
{
    // ximagesink only supports rgb and not yuv colorspace, so we need a converter here
    sink_ = Pipeline::Instance()->makeElement("ximagesink", NULL);
    prepareSink();

    gstlinkable::link(colorspc_, sink_);
}

XImageSink::~XImageSink()
{
    VideoSink::destroySink();
    Pipeline::Instance()->remove(&colorspc_);
}

