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

#include <gst/interfaces/xoverlay.h>
#include "gstLinkable.h"
#include "videoSink.h"
#include "pipeline.h"
#include "rtpReceiver.h"
#include "gtk_utils.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkcursor.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>


/// true if we're not using some external xwindow
bool GtkVideoSink::hasWindow() const
{
    return xid_ == 0;
}

void GtkVideoSink::updateDisplay(const std::string &display)
{
    GdkDisplay *disp = gdk_display_open(display.c_str());
    if (disp == 0)
        THROW_ERROR("Could not open display " << display);
    /// FIXME: should be able to get other screens than 0
    gtk_window_set_screen(GTK_WINDOW(window_), gdk_display_get_default_screen(disp));
}
        

GtkVideoSink::GtkVideoSink(const Pipeline &pipeline, unsigned long xid) : 
    VideoSink(pipeline), 
    xid_(xid),
    isFullscreen_(false),
    window_(hasWindow() ? gtk_window_new(GTK_WINDOW_TOPLEVEL) : 0), 
    drawingArea_(gtk_drawing_area_new()),
	vbox_(hasWindow() ? gtk_vbox_new(FALSE, 0) : 0),
	hbox_(hasWindow() ? gtk_hbox_new(FALSE, 0) : 0),
	horizontalSlider_(0),
	sliderFrame_(0)
{
    gtk_widget_set_double_buffered(drawingArea_, FALSE);
    if (hasWindow())
    {
        gtk_box_pack_start(GTK_BOX(hbox_), vbox_, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox_), drawingArea_, TRUE, TRUE, 0);

        gtk_container_add(GTK_CONTAINER(window_), hbox_);
        std::string iconPath(std::string(PIXMAPS_DIR) + "/scenic.png");
        // This test isn't very reliable since the icon file could be moved 
        // in between the test and the function call.
        if (g_file_test(iconPath.c_str(), G_FILE_TEST_EXISTS))
            gtk_window_set_icon_from_file(GTK_WINDOW(window_), iconPath.c_str(), NULL);

        // add listener for window-state-event to detect fullscreenness
        g_signal_connect(G_OBJECT(window_), "window-state-event", G_CALLBACK(onWindowStateEvent), this);
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
        LOG_DEBUG("Created plug with ID: " << static_cast<unsigned int>(gtk_plug_get_id(GTK_PLUG(plug))));
    }
}


gboolean GtkVideoSink::onWindowStateEvent(GtkWidget * /*widget*/, GdkEventWindowState *event, gpointer data)
{
    GtkVideoSink *context = static_cast<GtkVideoSink*>(data);
    context->isFullscreen_ = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    if (context->isFullscreen_)
        context->hideCursor();
    else
        context->showCursor();
    return TRUE;
}


Window GtkVideoSink::getXWindow()
{ 
    // FIXME: see https://bugzilla.gnome.org/show_bug.cgi?id=599885
    return GDK_WINDOW_XWINDOW(drawingArea_->window);
}


void GtkVideoSink::destroy_cb(GtkWidget * /*widget*/, gpointer data)
{

    LOG_DEBUG("Window closed, quitting.");
    GtkVideoSink *context = static_cast<GtkVideoSink*>(data);
    context->pipeline_.quit();
    context->window_ = 0;
}


void GtkVideoSink::makeDrawingAreaBlack()
{
    GdkColor color;
    gdk_color_parse ("black", &color);
    gtk_widget_modify_bg(drawingArea_, GTK_STATE_NORMAL, &color);    // needed to ensure black background
}


void GtkVideoSink::showWindow()
{
    makeDrawingAreaBlack();
    gtk_widget_show_all(window_);
}


void GtkVideoSink::hideCursor()
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


void GtkVideoSink::showCursor()
{
    /// sets to default
    gdk_window_set_cursor(GDK_WINDOW(drawingArea_->window), NULL);
}

void GtkVideoSink::toggleFullscreen(GtkWidget *widget)
{
    // toggle fullscreen state
    isFullscreen_ ? makeUnfullscreen(widget) : makeFullscreen(widget);
}


void GtkVideoSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget));           // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
    if (horizontalSlider_)
        gtk_widget_hide(horizontalSlider_);
    if (sliderFrame_)
        gtk_widget_hide(sliderFrame_);
}


void GtkVideoSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget));           // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
    /// show controls
    if (horizontalSlider_)
        gtk_widget_show(horizontalSlider_);
    if (sliderFrame_)
        gtk_widget_show(sliderFrame_);
}


bool GtkVideoSink::handleMessage(const std::string &path, const std::string &arguments)
{
    if (path == "fullscreen")
    {
        if (hasWindow())
            toggleFullscreen();
        return true;
    }
    else if (path == "window-title")
    {
        if (hasWindow())
            gtk_window_set_title(GTK_WINDOW(window_), arguments.c_str());
        return true;
    }
    else if (path == "create-control")
    {
        if (hasWindow())
            createControl();
        return true;
    }

    return false;
}


/* makes the latency window */
void GtkVideoSink::createControl()
{
    LOG_INFO("Creating controls");
    sliderFrame_ = gtk_frame_new("Jitterbuffer Latency (ms)");
    // min, max, step-size
    horizontalSlider_ = gtk_hscale_new_with_range(RtpReceiver::MIN_LATENCY, RtpReceiver::MAX_LATENCY, 1.0);

    // set initial value
    gtk_range_set_value(GTK_RANGE(horizontalSlider_), RtpReceiver::INIT_LATENCY);

    gtk_container_add(GTK_CONTAINER(sliderFrame_), horizontalSlider_);
    gtk_box_pack_start(GTK_BOX(vbox_), sliderFrame_, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(horizontalSlider_), "value-changed",
            G_CALLBACK(RtpReceiver::updateLatencyCb), NULL);
    showWindow();
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
                context->VideoSink::pipeline_.quit();
            }
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

    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), getXWindow());

    LOG_DEBUG("Got prepare-xwindow-id msg");
    return true;
}

XvImageSink::XvImageSink(Pipeline &pipeline, int width, int height, unsigned long xid, const std::string &display) : 
    GtkVideoSink(pipeline, xid),
    BusMsgHandler(&pipeline)
{
    sink_ = VideoSink::pipeline_.makeElement("xvimagesink", NULL);
    g_object_set(sink_, "force-aspect-ratio", TRUE, NULL);
    if (not display.empty())
    {
        g_object_set(sink_, "display", display.c_str(), NULL);
        updateDisplay(display);
    }
    if (hasWindow())
    {
        LOG_DEBUG("Setting default window size to " << width << "x" << height);
        gtk_window_set_default_size(GTK_WINDOW(window_), width, height);
        //gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

        gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
        g_signal_connect(G_OBJECT(window_), "key-press-event",
                G_CALLBACK(XvImageSink::key_press_event_cb), this);
        g_signal_connect(G_OBJECT(window_), "destroy",
                G_CALLBACK(destroy_cb), static_cast<gpointer>(this));

        gtk_widget_set_size_request(drawingArea_, width, height);
        showWindow();
    }
}


XvImageSink::~XvImageSink()
{
    if (hasWindow() and window_ != 0)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Videosink window destroyed");
    }
}


XImageSink::XImageSink(const Pipeline &pipeline, const std::string &display) : 
    VideoSink(pipeline),
    colorspc_(pipeline_.makeElement("ffmpegcolorspace", NULL)) 
{
    // ximagesink only supports rgb and not yuv colorspace, so we need a converter here
    sink_ = pipeline_.makeElement("ximagesink", NULL);
    g_object_set(sink_, "force-aspect-ratio", TRUE, NULL);
    if (not display.empty())
        g_object_set(sink_, "display", display.c_str(), NULL);

    gstlinkable::link(colorspc_, sink_);
}

