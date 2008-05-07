// Encode live dv in to x264 then packetize into RTP
//
// compile with:
// gcc -Wall $(pkg-config --cflags --libs gstreamer-0.10) cam.c -o cam

#include <string.h> /* for memset () */
#include <gst/gst.h>

static void
cb_handoff (GstElement *fakesink,
        GstBuffer  *buffer,
        GstPad     *pad,
        gpointer    user_data)
{
    /* TODO: send buffer from here? */
    memset (GST_BUFFER_DATA (buffer), 0,
            GST_BUFFER_SIZE (buffer));
}

gint
main (gint   argc,
      gchar *argv[])
{
    GstElement *pipeline, *fakesink;
    GMainLoop *loop;
    GError **error;

    /* init GStreamer */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* setup pipeline */
    pipeline = gst_parse_launch("dv1394src ! dvdemux ! dvdec ! \
                                ffmpegcolorspace ! x264enc threads=4 \
                                ! rtph264pay", error);

    fakesink = gst_element_factory_make("fakesink", "sink");
    
    /* setup fake sink */
    g_object_set (G_OBJECT (fakesink), "signal-handoffs", TRUE, NULL);
    g_signal_connect (fakesink, "handoff", G_CALLBACK (cb_handoff), NULL);

    /* setup */
    /*
    g_object_set (G_OBJECT (flt), "caps",
            gst_caps_new_simple ("video/x-raw-rgb",
                "width", G_TYPE_INT, 384,
                "height", G_TYPE_INT, 288,
                "framerate", GST_TYPE_FRACTION, 1, 1,
                "bpp", G_TYPE_INT, 16,
                "depth", G_TYPE_INT, 16,
                "endianness", G_TYPE_INT, G_BYTE_ORDER,
                NULL), NULL);*/
    gst_bin_add_many(GST_BIN (pipeline), fakesink, NULL);


    /* play */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));

    return 0;
}

