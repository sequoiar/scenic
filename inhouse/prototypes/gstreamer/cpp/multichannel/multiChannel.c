/* 
 * multiChannel.c
 *
 * Plays 8 channels of sine waves (increasing in frequency), equivalent to gst-launch line:
 * gst-launch-0.10 -v interleave name=i ! audioconvert  ! audioresample ! queue ! jackaudiosink \
 *                    audiotestsrc volume=0.125 freq=200 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=300 is-live=true ! audioconvert  ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=400 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=500 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=600 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=700 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=800 is-live=true ! audioconvert ! queue ! i. \
 *                    audiotestsrc volume=0.125 freq=900 is-live=true ! audioconvert ! queue ! i. 
 *
 */

#include <gst/gst.h>
#include <gst/audio/multichannel.h>

#define NUM_CHANNELS 8

#if 0
static const GstAudioChannelPosition NONE_POSITIONS[NUM_CHANNELS] = {
      GST_AUDIO_CHANNEL_POSITION_NONE, 
      GST_AUDIO_CHANNEL_POSITION_NONE, 
      GST_AUDIO_CHANNEL_POSITION_NONE,
      GST_AUDIO_CHANNEL_POSITION_NONE, 
      GST_AUDIO_CHANNEL_POSITION_NONE, 
      GST_AUDIO_CHANNEL_POSITION_NONE, 
      GST_AUDIO_CHANNEL_POSITION_NONE, 
      GST_AUDIO_CHANNEL_POSITION_NONE
};
#endif


static const GstAudioChannelPosition CHANNEL_POSITIONS[][NUM_CHANNELS] = {
    {                           /* Mono */
        GST_AUDIO_CHANNEL_POSITION_FRONT_MONO
    },
    {                          /* Stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                          /* Stereo + Centre */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT
    },
    {                          /* Quadraphonic */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT
    },
    {                          /* Stereo + Centre + rear stereo */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT
        ,
    },
    {                          /* Full 5.1 Surround */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_LFE
    },
    {                          /* Not defined by spec, GStreamer default */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE,
        GST_AUDIO_CHANNEL_POSITION_REAR_CENTER
    },
    {                          /* Not defined by spec, GStreamer default */
        GST_AUDIO_CHANNEL_POSITION_FRONT_LEFT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_REAR_LEFT,
        GST_AUDIO_CHANNEL_POSITION_REAR_RIGHT,
        GST_AUDIO_CHANNEL_POSITION_FRONT_CENTER,
        GST_AUDIO_CHANNEL_POSITION_LFE,
        GST_AUDIO_CHANNEL_POSITION_SIDE_LEFT,
        GST_AUDIO_CHANNEL_POSITION_SIDE_RIGHT
    }
    ,
};


// For many plugins (vorbisenc, alsasink, etc.) the position of each channel must be specified for the pipeline to roll.
static void set_channel_layout(GstElement *interleave)
{
    GValue val = { 0, };
    GValueArray *arr;           // for channel position layout
    arr = g_value_array_new(NUM_CHANNELS);

    g_object_set(interleave, "channel-positions-from-input", FALSE, NULL);

    g_value_init(&val, GST_TYPE_AUDIO_CHANNEL_POSITION);

    int channelIdx;
    for (channelIdx = 0; channelIdx < NUM_CHANNELS; channelIdx++)
    {
        g_value_set_enum(&val, CHANNEL_POSITIONS[NUM_CHANNELS - 1][channelIdx]);
        g_value_array_append(arr, &val);
        g_value_reset(&val);
    }

    g_value_unset(&val);

    g_object_set(interleave, "channel-positions", arr, NULL);
    g_value_array_free(arr);
}


static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
//    GMainLoop *loop = data;
    gboolean *done = (gboolean*) data;

    switch(GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_EOS:
            g_print("End-of-stream\n");
            *done = TRUE;
            
            //g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: 
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_error(msg, &err, &debug);

                g_print("Error: %s\n", err->message);
                g_error_free(err);

                if (debug) {
                    g_print("Debug details: %s\n", debug);
                    g_free(debug);
                }

                *done = TRUE;
                //g_main_loop_quit(loop);
                break;
            }
        case GST_MESSAGE_STATE_CHANGED:
            //g_print("state changed \n");
            break;
        default:
            break;
    }

    return TRUE;
}


#if 0
static GstCaps *
generate_multichannel_src_caps()
{
  GstCaps *caps = gst_caps_new_simple ("audio/x-raw-int",
          "endianness", G_TYPE_INT, G_BIG_ENDIAN,
          "signed", G_TYPE_BOOLEAN, TRUE,
          "width", G_TYPE_INT, 16,
          "depth", G_TYPE_INT, 16,
          "rate", G_TYPE_INT, 44100, "channels", G_TYPE_INT, NUM_CHANNELS, NULL);

  gst_audio_set_caps_channel_positions_list(caps, NONE_POSITIONS, NUM_CHANNELS);
  g_assert(caps);
  g_print("Here are your caps %s\n", gst_caps_to_string(caps));
  return caps;
}
#endif


gint main(gint argc, gchar *argv[])
{
    GstStateChangeReturn ret;
    const char *err_str = " could not be found - check your install\n";
    GstElement *pipeline, *interleave, *audioresample, *sink;
    GstElement *sources[NUM_CHANNELS];
    GstElement *audioconverts[NUM_CHANNELS + 1];
    GstElement *queues[NUM_CHANNELS + 1];
    //GstElement *payAconv, *depayAconv, *rtpL16pay, *rtpL16depay, *capsSetter;
    gboolean done = FALSE;
    GMainLoop *loop;
    GstBus *bus;

    /* initialization */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    /* create elements */
    pipeline = gst_pipeline_new("audioPipeline");
    g_signal_connect(pipeline, "deep_notify",
            G_CALLBACK(gst_object_default_deep_notify), NULL);

    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, &done);
    gst_object_unref(bus);

    if (!(interleave = gst_element_factory_make("interleave", NULL)))
        g_print("interleave%s", err_str);

    set_channel_layout(interleave);

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
#if 0
    if (!(payAconv = gst_element_factory_make("audioconvert", NULL)))
        g_print("payAconv%s", err_str);
    if (!(rtpL16pay = gst_element_factory_make("rtpL16pay", NULL)))
        g_print("rtpL16pay%s", err_str);
    if (!(rtpL16depay = gst_element_factory_make("rtpL16depay", NULL)))
        g_print("rtpL16depay%s", err_str);
    if (!(capsSetter = gst_element_factory_make("capssetter", NULL)))
        g_print("capsSetter%s", err_str);
    if (!(depayAconv = gst_element_factory_make("audioconvert", NULL)))
        g_print("depayaconv%s", err_str);
#endif
    if (!(sink = gst_element_factory_make("jackaudiosink", NULL)))
        g_print("sink%s", err_str);

    g_object_set(G_OBJECT(sink), "connect", 1, NULL);   // autoconnect jack ports
    g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);

#if 0
    GstCaps *depaySrcCaps = generate_multichannel_src_caps();
    g_assert(depaySrcCaps);
    g_object_set(G_OBJECT(capsSetter), "caps", depaySrcCaps, NULL); // add multichannel caps
#endif

    gst_bin_add_many(GST_BIN (pipeline), audioconverts[NUM_CHANNELS], audioresample, queues[NUM_CHANNELS], /*payAconv,
            rtpL16pay, rtpL16depay, capsSetter, depayAconv, */sink, NULL);

    if (!(gst_element_link_many(interleave, audioconverts[NUM_CHANNELS], audioresample, queues[NUM_CHANNELS], /*payAconv, 
                    rtpL16pay, rtpL16depay, capsSetter, depayAconv, */sink, NULL)))
    {
        g_print("Failed to link. Quitting.\n");
        return -1;
    }

    /* run */
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
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

    //gst_caps_unref(depaySrcCaps);

    return 0;
}

