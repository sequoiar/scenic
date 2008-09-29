/* GStreamer
 * Copyright (C) 2000,2001,2002,2003,2005
 *           Thomas Vander Stichele <thomas at apestaart dot org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <math.h>

#include <gst/gst.h>

#include "levels.h"

void updateRms(GstMessage *message, gpointer context)
{
    VuMeter *meter = static_cast<VuMeter*>(context);
    const GstStructure *s = gst_message_get_structure (message);
    gint channels;
    //GstClockTime endtime;
    gdouble rms_dB, peak_dB, decay_dB;
    const GValue *list;
    const GValue *value;

    gint i;

    //   if (!gst_structure_get_clock_time (s, "endtime", &endtime))
    //      g_warning ("Could not parse endtime");
    /* we can get the number of channels as the length of any of the value
     * lists */
    list = gst_structure_get_value (s, "rms");
    channels = gst_value_list_get_size (list);

    //g_print ("endtime: %" GST_TIME_FORMAT ", channels: %d\n",
    //   GST_TIME_ARGS (endtime), channels);
    for (i = 0; i < channels; ++i) {
        //g_print ("channel %d\n", i);
        list = gst_structure_get_value (s, "rms");
        value = gst_value_list_get_value (list, i);
        rms_dB = g_value_get_double (value);
        list = gst_structure_get_value (s, "peak");
        value = gst_value_list_get_value (list, i);
        peak_dB = g_value_get_double (value);
        list = gst_structure_get_value (s, "decay");
        value = gst_value_list_get_value (list, i);
        decay_dB = g_value_get_double (value);
        //g_print ("    RMS: %f dB, peak: %f dB, decay: %f dB\n",
        //   rms_dB, peak_dB, decay_dB);

        /* converting from dB to normal gives us a value between 0.0 and 1.0 */
        meter->updateRms(pow(10, rms_dB * 0.05));
        //g_print ("    normalized rms value: %f\n", rms);
    }
}

// handler for msgs of type Gst_Message_Element
    gboolean
cb_message_element(GstBus *bus, GstMessage *message, gpointer data)
{
    const GstStructure *s = gst_message_get_structure (message);
    const gchar *name = gst_structure_get_name (s);

    if (strcmp (name, "level") == 0) {
        updateRms(message, data);
    }

    return TRUE;
}


    int
main (int argc, char *argv[])
{
    GstElement *jackaudiosrc, *level, *jackaudiosink;
    GstElement *pipeline;
    GstBus *bus;
    GMainLoop *loop; VuMeter meter;

    gst_init (&argc, &argv);

    pipeline = gst_pipeline_new (NULL);
    g_assert (pipeline);
    jackaudiosrc = gst_element_factory_make ("jackaudiosrc", NULL);
    g_assert (jackaudiosrc);
    level = gst_element_factory_make ("level", NULL);
    g_assert (level);
    jackaudiosink = gst_element_factory_make ("jackaudiosink", NULL);
    g_assert (jackaudiosink);
    g_object_set(G_OBJECT(jackaudiosink), "sync", FALSE, NULL);

    gst_bin_add_many (GST_BIN (pipeline), jackaudiosrc, level, 
            jackaudiosink, NULL);
    g_assert (gst_element_link_many(jackaudiosrc, level, jackaudiosink, NULL));

    /* make sure we'll get messages */
    g_object_set (G_OBJECT (level), "message", TRUE, NULL);
    guint64 interval = 999999999;
    g_object_set (G_OBJECT (level), "interval", interval, NULL);
    guint64 temp;
    g_object_get(G_OBJECT(level), "interval", &temp, NULL);
    g_assert(temp == interval);

    bus = gst_element_get_bus (pipeline);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message::element", G_CALLBACK(cb_message_element), static_cast<void*>(&meter));

    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    // add callback 

    /* we need to run a GLib main loop to get the messages */
    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    return 0;
}


