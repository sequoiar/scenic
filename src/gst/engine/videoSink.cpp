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
#include "videoSize.h"
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

void VideoSink::defaultHandler(const std::string &message)
{
    LOG_ERROR("Unimplemented handler for message " << message);
}

Window GtkVideoSink::getXWindow()
{ 
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
    gtk_window_set_title(GTK_WINDOW(window_), "Milhouse");
    gtk_widget_show_all(window_);
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


void GtkVideoSink::handleMessage(const std::string &message)
{
    if (message == "fullscreen")
        toggleFullscreen();
    else
        VideoSink::defaultHandler(message);
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
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), GDK_WINDOW_XWINDOW(window_->window));
  
    return true;
}


void XvImageSink::init()
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

    GdkDisplay* display = gdk_display_get_default();
    tassert(display);
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
            gtk_window_move(GTK_WINDOW(window_), xine[j].x_org, xine[j].y_org);
    }

    gtk_window_set_default_size(GTK_WINDOW(window_), videosize::WIDTH, videosize::HEIGHT);
    //gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
            G_CALLBACK(XvImageSink::key_press_event_cb), NULL);
    g_signal_connect(G_OBJECT(window_), "destroy",
            G_CALLBACK(destroy_cb), static_cast<gpointer>(this));

    showWindow();
    // register this level to handle prepare window id msg
    Pipeline::Instance()->subscribe(this);
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


void XImageSink::init()
{
    // ximagesink only supports rgb and not yuv colorspace, so we need a converter here
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", NULL);
    sink_ = Pipeline::Instance()->makeElement("ximagesink", NULL);
    prepareSink();

    gstlinkable::link(colorspc_, sink_);
}


XImageSink::~XImageSink()
{
    VideoSink::destroySink();
    Pipeline::Instance()->remove(&colorspc_);
}


void XImageSink::handleMessage(const std::string &message)
{
    VideoSink::defaultHandler(message);
}

