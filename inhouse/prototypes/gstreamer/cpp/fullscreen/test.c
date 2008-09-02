#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gst/interfaces/xoverlay.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

static gboolean expose_cb(GtkWidget * widget, GdkEventExpose * event, gpointer data)
{
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(data), GDK_WINDOW_XWINDOW(widget->window));
    //gtk_widget_show_all(widget);
    return TRUE;
}

#if 0
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = data;

    switch (GST_MESSAGE_TYPE (msg)) 
    {
        case GST_MESSAGE_EOS:
            g_print ("End-of-stream\n");
            g_main_loop_quit (loop);
            break;
        case GST_MESSAGE_ERROR: 
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_error (msg, &err, &debug);

                g_print ("Error: %s\n", err->message);
                g_error_free (err);

                if (debug) {
                    g_print ("Debug deails: %s\n", debug);
                    g_free (debug);
                }

                g_main_loop_quit (loop);
                break;
            }
        default:
            break;
    }

    return TRUE;
}
#endif

static gboolean
key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (event->keyval != 'f')
        return TRUE;
    else 
        g_print("you hit f\n");

    gboolean isFullscreen = (gdk_window_get_state(GDK_WINDOW(widget->window)) == GDK_WINDOW_STATE_FULLSCREEN);

    if (isFullscreen) 
        gtk_window_unfullscreen(GTK_WINDOW(widget));
    else
        gtk_window_fullscreen(GTK_WINDOW(widget));

    return TRUE;
}

#if 0
static GstBusSyncReply
create_window (GstBus * bus, GstMessage * message, GstPipeline * pipeline)
{
    if (window)
        return GST_BUS_PASS;

//    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
 //   gtk_widget_show(window);


  const GstStructure *s;
  GstXOverlay *ov = NULL;

  s = gst_message_get_structure (message);
  if (!gst_structure_has_name (s, "prepare-xwindow-id")) 
    return GST_BUS_PASS;

  ov = GST_X_OVERLAY (GST_MESSAGE_SRC (message));

  g_print ("Creating our own window %d\n", GDK_WINDOW_XID(window));

  gst_x_overlay_set_xwindow_id(ov, GDK_WINDOW_XID(window));

  return GST_BUS_DROP;
}
#endif


gint main (gint argc, gchar *argv[])
{
    GstStateChangeReturn ret;
    GstElement *pipeline, *src, *sink;
    GMainLoop *loop;
    //GstBus *bus;
    GtkWidget *window;

    /* initialization */
    gst_init (&argc, &argv);
    gtk_init (&argc, &argv);

    loop = g_main_loop_new (NULL, FALSE);

    /* create elements */
    pipeline = gst_pipeline_new ("my_pipeline");

#if 0
    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);
#endif

    src = gst_element_factory_make ("v4l2src", NULL);

    sink = gst_element_factory_make("xvimagesink", "videosink");
    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);


    if (!sink)
        g_print ("output could not be found - check your install\n");

    gst_bin_add_many (GST_BIN (pipeline), src, sink, NULL);
    g_object_set(G_OBJECT(sink), "force-aspect-ratio", TRUE, NULL);

    //GstElement *temp = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
//    g_assert(temp);

    /* link everything together */
    if (!gst_element_link(src, sink)) {
        g_print ("Failed to link one or more elements!\n");
        return -1;
    }
    //gst_bus_set_sync_handler(bus, (GstBusSyncHandler)create_window, pipeline);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "expose-event", G_CALLBACK(expose_cb), sink);

    gtk_widget_set_events(window, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(key_press_event_cb), sink);

    //g_signal_connect(G_OBJECT(window), "realize", G_CALLBACK(realize_cb), sink);
    //gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(sink), GDK_WINDOW_XID(window->window));


    /* run */
    gtk_widget_show_all(window);

    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("Failed to start up pipeline!\n");

#if 0
        /* check if there is an error message with details on the bus */
        GstMessage *msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
        if (msg) {
            GError *err = NULL;

            gst_message_parse_error (msg, &err, NULL);
            g_print ("ERROR: %s\n", err->message);
            g_error_free (err);
            gst_message_unref (msg);
        }
        return -1;
#endif
    }

    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;
}
