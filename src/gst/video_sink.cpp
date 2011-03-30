/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gst/interfaces/xoverlay.h>
#include "gst_linkable.h"
#include "video_sink.h"
#include "pipeline.h"
#include "gutil/gutil.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkcursor.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

/// true if we're not using some external xwindow
bool XvImageSink::hasWindow() const
{
    return xid_ == 0;
}

void XvImageSink::updateDisplay(const std::string &display)
{
    GdkDisplay *disp = gdk_display_open(display.c_str());
    if (disp == 0)
        THROW_ERROR("Could not open display " << display);
    /// FIXME: should be able to get other screens than 0
    gtk_window_set_screen(GTK_WINDOW(window_), gdk_display_get_default_screen(disp));
}

namespace {
void makeWidgetBlack(GtkWidget *widget)
{
    GdkColor black;
    gdk_color_parse ("Black", &black);
    gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, &black);
    gtk_widget_modify_bg (widget, GTK_STATE_ACTIVE, &black);
    gtk_widget_modify_bg (widget, GTK_STATE_PRELIGHT, &black);
    gtk_widget_modify_bg (widget, GTK_STATE_SELECTED, &black);
    gtk_widget_modify_bg (widget, GTK_STATE_INSENSITIVE, &black);
}
}

XvImageSink::XvImageSink(Pipeline &pipeline,
        int width,
        int height,
        unsigned long xid,
        const std::string &display,
        const std::string &title) :
    VideoSink(),
    xid_(xid),
    isFullscreen_(false),
    window_(hasWindow() ? gtk_window_new(GTK_WINDOW_TOPLEVEL) : 0),
    drawingArea_(gtk_drawing_area_new()),
	vbox_(hasWindow() ? gtk_vbox_new(FALSE, 0) : 0),
	hbox_(hasWindow() ? gtk_hbox_new(FALSE, 0) : 0),
	horizontalSlider_(0),
	sliderFrame_(0)
{
    // don't set widget size, it needs to be resized dynamically
    //gtk_widget_set_size_request(drawingArea_, width, height);

    // Make drawing area black by default, since it's used for video
    makeWidgetBlack(drawingArea_);

    sink_ = pipeline.makeElement("xvimagesink", NULL);
    g_object_set(sink_, "force-aspect-ratio", TRUE, NULL);
    if (not display.empty())
    {
        g_object_set(sink_, "display", display.c_str(), NULL);
        updateDisplay(display);
    }

    gtk_widget_set_double_buffered(drawingArea_, FALSE);
    if (hasWindow())
    {
        gtk_window_set_title(GTK_WINDOW(window_), title.c_str());
        LOG_DEBUG("Setting default window size to " << width << "x" << height);
        gtk_window_set_default_size(GTK_WINDOW(window_), width, height);
        gtk_box_pack_start(GTK_BOX(hbox_), vbox_, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox_), drawingArea_, TRUE, TRUE, 0);

        gtk_container_add(GTK_CONTAINER(window_), hbox_);

        // set icon
        std::string iconPath(std::string(PIXMAPS_DIR) + "/scenic.png");
        if (gtk_window_set_icon_from_file(GTK_WINDOW(window_), iconPath.c_str(), NULL))
            LOG_DEBUG("Using icon " << iconPath << " for window");
        else
            LOG_DEBUG(iconPath << " does not exist");

        // add listener for window-state-event to detect fullscreenness
        g_signal_connect(G_OBJECT(window_), "window-state-event", G_CALLBACK(onWindowStateEvent), this);
        g_signal_connect (G_OBJECT (window_), "delete-event",
                G_CALLBACK (window_closed), this);
        // grab key press events
        gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
        g_signal_connect(G_OBJECT(window_), "key-press-event",
                G_CALLBACK(XvImageSink::key_press_event_cb), this);

        gtk_widget_show_all (window_);
        gtk_widget_realize (window_);
    }
    else
    {
        /* make plug */
        GtkWidget *plug = gtk_plug_new(xid_);
        gtk_container_add(GTK_CONTAINER(plug), drawingArea_);
        /* end main loop when plug is destroyed */
        g_signal_connect(G_OBJECT (plug), "destroy", G_CALLBACK(gutil::killMainLoop), NULL);
        /* show window and log its id */
        gtk_widget_show_all(plug);
        gtk_widget_realize (plug);
        LOG_DEBUG("Created plug with ID: " << static_cast<unsigned int>(gtk_plug_get_id(GTK_PLUG(plug))));
    }

    GdkWindow *drawingAreaXWindow = gtk_widget_get_window (drawingArea_);
    gulong embed_xid = GDK_WINDOW_XID (drawingAreaXWindow);
    gst_x_overlay_set_xwindow_id (GST_X_OVERLAY (sink_), embed_xid);
}

gboolean XvImageSink::onWindowStateEvent(GtkWidget * /*widget*/, GdkEventWindowState *event, gpointer data)
{
    XvImageSink *context = static_cast<XvImageSink*>(data);
    context->isFullscreen_ = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    if (context->isFullscreen_)
        context->hideCursor();
    else
        context->showCursor();
    return TRUE;
}

void XvImageSink::window_closed(GtkWidget * widget, GdkEvent * /*event*/, gpointer data)
{
    LOG_DEBUG("Window closed, quitting.");
    gtk_widget_hide_all (widget);
    XvImageSink *context = static_cast<XvImageSink*>(data);
    gutil::killMainLoop();
    context->window_ = 0;
}

void XvImageSink::hideCursor()
{
    static GdkCursor* cursor = 0;

    if (cursor == 0)
    {
        // GDK_BLANK_CURSOR is available in gtk-2.16 and later
#if GTK_CHECK_VERSION (2,16,0)
        cursor = gdk_cursor_new(GDK_BLANK_CURSOR);
#else
        char invisible_cursor_bits[] = { 0x0 };
        static GdkBitmap *empty_bitmap;
        const static GdkColor color = {0, 0, 0, 0};
        empty_bitmap = gdk_bitmap_create_from_data(GDK_WINDOW(drawingArea_->window),
                invisible_cursor_bits,
                1, 1);
        cursor = gdk_cursor_new_from_pixmap(empty_bitmap, empty_bitmap, &color,
                &color, 0, 0);
#endif
    }

    gdk_window_set_cursor(GDK_WINDOW(drawingArea_->window), cursor);
}


void XvImageSink::showCursor()
{
    /// sets to default
    gdk_window_set_cursor(GDK_WINDOW(drawingArea_->window), NULL);
}

void XvImageSink::toggleFullscreen(GtkWidget *widget)
{
    // toggle fullscreen state
    isFullscreen_ ? makeUnfullscreen(widget) : makeFullscreen(widget);
}


void XvImageSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget));           // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
    if (horizontalSlider_)
        gtk_widget_hide(horizontalSlider_);
    if (sliderFrame_)
        gtk_widget_hide(sliderFrame_);
}


void XvImageSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget));           // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
    // show controls
    if (horizontalSlider_)
        gtk_widget_show(horizontalSlider_);
    if (sliderFrame_)
        gtk_widget_show(sliderFrame_);
}


void XvImageSink::toggleFullscreen()
{
    toggleFullscreen(window_);
}

gboolean XvImageSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    XvImageSink *context = static_cast<XvImageSink*>(data);
    switch (event->keyval)
    {
        case GDK_f:
        case GDK_F:
        case GDK_Escape:
            context->toggleFullscreen(widget);
            break;

        case GDK_q:
            // Quit application on ctrl-q, this quits the main loop
            // (if there is one)
            if (event->state & GDK_CONTROL_MASK)
            {
                LOG_INFO("Ctrl-Q key pressed, quitting.");
                gutil::killMainLoop();
            }
            break;

        default:
            break;
    }

    return TRUE;
}

XvImageSink::~XvImageSink()
{
    if (hasWindow() and window_ != 0)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Videosink window destroyed");
    }
}


// FIXME: this should be refactored to use the xoverlay interface/gtk stuff
XImageSink::XImageSink(const Pipeline &pipeline, const std::string &display) :
    VideoSink(),
    colorspace_(pipeline.makeElement("ffmpegcolorspace", NULL))
{
    // ximagesink only supports rgb and not yuv colorspace, so we need a converter here
    sink_ = pipeline.makeElement("ximagesink", NULL);
    g_object_set(sink_, "force-aspect-ratio", TRUE, NULL);
    if (not display.empty())
        g_object_set(sink_, "display", display.c_str(), NULL);

    gstlinkable::link(colorspace_, sink_);
}

GstElement *XImageSink::sinkElement()
{
    return colorspace_;
}
