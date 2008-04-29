
/************************************************ 
 * A simple program to get dv input from a camera
 * over firewire, and output the sound and video.
 * Built based on gstreamer examples.
 * 
 * compile with: 
 * gcc -Wall $(pkg-config --cflags --libs gstreamer-0.10) cam.c -o cam
************************************************/ 

#include <gst/gst.h>

/*
 * Global objects are usually a bad thing. For the purpose of this
 * example, we will use them, however.
 */

GstElement *pipeline;

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) 
  {
      case GST_MESSAGE_EOS:
          g_print ("End-of-stream\n");
          g_main_loop_quit (loop);
          break;

      case GST_MESSAGE_ERROR: 
          {
              gchar *debug;
              GError *err;

              gst_message_parse_error (msg, &err, &debug);
              g_free (debug);

              g_print ("Error: %s\n", err->message);
              g_error_free (err);

              g_main_loop_quit (loop);
              break;
          }
      default:
          break;
  }

  return TRUE;
}

int main (int argc, char *argv[])
{
    GMainLoop *loop;
    GstBus *bus;
    GError** error;

    /* initialize GStreamer */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* create elements */
    pipeline = gst_parse_launch("dv1394src ! dvdemux name=demux demux. ! \
                                 queue ! dvdec ! xvimagesink sync=false \
                                 demux. ! queue ! audioconvert ! \
                                 alsasink sync=false", error);

    if (!pipeline)
    {
        g_print ("Pipeline could not be created\n");
        return -1;
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);
           
    /* Now set to playing and iterate. */
    g_print ("Setting to PLAYING\n");
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_print ("Running\n");
    g_main_loop_run (loop);

    /* clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);
    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));

    return 0;
}

