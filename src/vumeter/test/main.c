/* main.c */

#define UNUSED(expr) do { (void)(expr); } while (0)

#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "vumeter.h"

static const gboolean LOG = FALSE;

static void
set_value (gdouble value, gpointer data)
{
  GdkRegion *region;

  GtkWidget *vumeter = (GtkWidget *) data;
  GTK_VUMETER (vumeter)->sel = value;

  region = gdk_drawable_get_clip_region (vumeter->window);
  gdk_window_invalidate_region (vumeter->window, region, TRUE);
  gdk_window_process_updates (vumeter->window, TRUE);
}

gboolean
message_handler (GstBus * bus, GstMessage * message, gpointer data)
{
  UNUSED (bus);
  if (message->type == GST_MESSAGE_ELEMENT) {
    const GstStructure *s = gst_message_get_structure (message);
    const gchar *name = gst_structure_get_name (s);

    if (strncmp (name, "level", strlen ("level")) == 0) {
      GtkWidget **vumeters = (GtkWidget **) data;
      gint channels;
      GstClockTime endtime;
      gdouble rms_dB, peak_dB, decay_dB;
      /*gdouble rms;*/
      gdouble peak;
      const GValue *list;
      const GValue *value;

      gint i;

      if (!gst_structure_get_clock_time (s, "endtime", &endtime))
        g_warning ("Could not parse endtime");
      /* we can get the number of channels as the length of any of the value
       * lists */
      list = gst_structure_get_value (s, "rms");
      channels = gst_value_list_get_size (list);

      for (i = 0; i < channels; ++i) {
        if (LOG)
          g_print ("channel %d\n", i);
        list = gst_structure_get_value (s, "rms");
        value = gst_value_list_get_value (list, i);
        rms_dB = g_value_get_double (value);
        list = gst_structure_get_value (s, "peak");
        value = gst_value_list_get_value (list, i);
        peak_dB = g_value_get_double (value);
        list = gst_structure_get_value (s, "decay");
        value = gst_value_list_get_value (list, i);
        decay_dB = g_value_get_double (value);
        if (1)
          g_print ("    RMS: %f dB, peak: %f dB, decay: %f dB\n",
              rms_dB, peak_dB, decay_dB);

        /* converting from dB to normal gives us a value between 0.0 and 1.0 */
        peak = pow (10, peak_dB / 20);
        if (LOG)
          g_print ("    normalized peak value: %f\n", peak);
        set_value (peak_dB, vumeters[i]);
      }
    }
  }
  /* we handled the message we want, and ignored the ones we didn't want.
   * so the core can unref the message for us */
  return TRUE;
}


void
embed_event (GtkWidget * plug)
{
  UNUSED (plug);
  g_print ("embedded!");
}

gboolean quit_cb(gpointer data)
{
    UNUSED(data);
    gtk_main_quit();
    return TRUE;
}

int
main (int argc, char **argv)
{
  GtkWidget *plug;
  enum { NUM_CHANNELS = 2 };
  GtkWidget *vumeters[NUM_CHANNELS];
  GtkWidget *hbox;
  GstBus *bus;
  gint watch_id;
  gboolean long_test = FALSE;
  GdkNativeWindow socket_id;  
  GstElement *pipeline;
  GstElement *source;
  GstElement *capsfilter;
  GstCaps *caps;
  /*GstElement *audioconvert;*/
  GstElement *level;
  GstElement *sink;
  gint i;
  enum
  { SINE = 0, SQUARE, SAW, TRIANGLE, SILENCE, WHITE_NOISE, PINK_NOISE,
    SINE_TABLE, TICKS, GAUSSIAN_NOISE
  };
  enum {SPACING = 5}; 

  if (argc < 2)
      socket_id = 0;
  else
      socket_id = atol(argv[1]);
  if (argc == 3)
      long_test = TRUE;

  gtk_init (&argc, &argv);
  gst_init (&argc, &argv);

  /* make frame with homogenous spacing*/
  hbox = gtk_hbox_new(TRUE, SPACING);
  /* make window */
  plug = gtk_plug_new (socket_id);

  /* end main loop when plug is destroyed */
  g_signal_connect (G_OBJECT (plug), "destroy", gtk_main_quit, NULL);

  for (i = 0; i < NUM_CHANNELS; ++i)
  {
    vumeters[i] = gtk_vumeter_new ();
    gtk_container_add (GTK_CONTAINER (hbox), vumeters[i]);
  }
    gtk_container_add (GTK_CONTAINER (plug), hbox);

  /* make pipeline */
  pipeline = gst_pipeline_new ("pipeline");
  g_signal_connect (G_OBJECT (plug), "embedded",
      G_CALLBACK (embed_event), NULL );
  source = gst_element_factory_make ("jackaudiosrc", NULL);
  /*g_object_set (source, "wave", TICKS, NULL);*/

  capsfilter= gst_element_factory_make ("capsfilter", NULL);
  /* 2 channels is sadly the max for audiotestsrc without interleaving */
  g_assert((caps = gst_caps_from_string("audio/x-raw-float, channels=2, rate=48000")));
  g_object_set(capsfilter, "caps", caps, NULL);
  gst_caps_unref(caps);

  /*audioconvert = gst_element_factory_make("audioconvert", NULL);*/
  level = gst_element_factory_make ("level", NULL);
  g_object_set(level, "interval", 50000000, NULL);
  sink = gst_element_factory_make ("fakesink", NULL);
  g_object_set(sink, "sync", TRUE, NULL);

  gst_bin_add_many (GST_BIN (pipeline), source, capsfilter, level, sink, NULL);
  gst_element_link_many (source, capsfilter, level, sink, NULL);

  bus = gst_element_get_bus (pipeline);
  watch_id = gst_bus_add_watch (bus, message_handler, vumeters);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* show window and run main loop */
  gtk_widget_show_all (plug);
  g_print ("%u\n", (unsigned int) gtk_plug_get_id (GTK_PLUG (plug)));
  /* we need to run a GLib main loop to get the messages */
  /* end in 200 ms */
  g_timeout_add(200, quit_cb, NULL); 
  gtk_main();

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (gst_pipeline_get_bus (GST_PIPELINE
              (pipeline))));
  gst_object_unref (GST_OBJECT (pipeline));
  g_print ("axed pipeline\n");

  return 0;
}
