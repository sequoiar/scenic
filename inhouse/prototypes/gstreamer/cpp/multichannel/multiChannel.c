/* 
 * multiChannel.c
 *
 * Plays 8 channels of sine waves (increasing in frequency), equivalent to gst-launch line:
 * gst-launch-0.10 -v interleave name=i ! audioconvert  ! queue ! jackaudiosink \
 *                    audiotestsrc volume=0.125 freq=200 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=300 is-live=true ! audioconvert  ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=400 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=500 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=600 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=700 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=800 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=900 is-live=true ! audioconvert ! queue ! i. 
 */

#include <gst/gst.h>

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = data;

    switch(GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_EOS:
            g_print("End-of-stream\n");
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: 
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_error(msg, &err, &debug);

                g_print("Error: %s\n", err->message);
                g_error_free(err);

                if (debug) {
                    g_print("Debug deails: %s\n", debug);
                    g_free(debug);
                }

                g_main_loop_quit(loop);
                break;
            }
        default:
            break;
    }

    return TRUE;
}


gint main(gint argc, gchar *argv[])
{
    GstStateChangeReturn ret;
    const char *err_str = " could not be found - check your install\n";
    GstElement *pipeline, *interleave, *audioresample, *sink;
    const int NUM_CHANNELS = 8;
    GstElement *sources[NUM_CHANNELS];
    GstElement *audioconverts[NUM_CHANNELS + 1];
    GstElement *queues[NUM_CHANNELS + 1];
    GMainLoop *loop;
    GstBus *bus;

    /* initialization */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    /* create elements */
    pipeline = gst_pipeline_new("audioPipeline");

    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    if (!(interleave = gst_element_factory_make("interleave", NULL)))
        g_print("interleave%s", err_str);

    gst_bin_add(GST_BIN(pipeline), interleave);
            
    const double GAIN = 1.0 / NUM_CHANNELS;
    const double FUNDAMENTAL = 200.0;

    int channel;
    for (channel = 0; channel < NUM_CHANNELS; channel++)
    {
        if (!(sources[channel] = gst_element_factory_make("audiotestsrc", NULL)))
            g_print("source%s", err_str);
        if (!(audioconverts[channel] = gst_element_factory_make("audioconvert", NULL)))
            g_print("audioconvert%s", err_str);
        if (!(queues[channel] = gst_element_factory_make("queue", NULL)))
            g_print("queue%s", err_str);
    
        gst_bin_add_many(GST_BIN(pipeline), sources[channel], audioconverts[channel], queues[channel], NULL);
        gst_element_link_many(sources[channel], audioconverts[channel], queues[channel], interleave, NULL);
            
        g_object_set(G_OBJECT(sources[channel]), "freq", (channel + 1.0) * FUNDAMENTAL, "volume", GAIN, NULL);
    }
    
    if (!(audioconverts[NUM_CHANNELS] = gst_element_factory_make("audioconvert", NULL)))
            g_print("audioconvert%s", err_str);
    if (!(queues[NUM_CHANNELS] = gst_element_factory_make("queue", NULL)))
            g_print("queue%s", err_str);
    if (!(audioresample = gst_element_factory_make("audioresample", NULL)))
            g_print("audioresample%s", err_str);
    if (!(sink = gst_element_factory_make("jackaudiosink", NULL)))
        g_print("sink%s", err_str);

    g_object_set(G_OBJECT(sink), "connect", 1, NULL);
    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
        
    gst_bin_add_many(GST_BIN (pipeline), audioconverts[NUM_CHANNELS], audioresample, queues[NUM_CHANNELS], sink);

    if (!(gst_element_link_many(interleave, audioconverts[NUM_CHANNELS], audioresample, queues[NUM_CHANNELS], sink)))
    {
        g_print("Failed to link. Quitting.\n");
        return -1;
    }

    /* run */
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("Failed to start up pipeline!\n");

        /* check if there is an error message with details on the bus */
        GstMessage *msg = gst_bus_poll(bus, GST_MESSAGE_ERROR, 0);
        if (msg) {
            GError *err = NULL;

            gst_message_parse_error(msg, &err, NULL);
            g_print("ERROR: %s\n", err->message);
            g_error_free(err);
            gst_message_unref(msg);
        }
        return -1;
    }

    g_main_loop_run(loop);

    /* clean up */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
