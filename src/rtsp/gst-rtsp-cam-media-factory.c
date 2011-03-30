/* GStreamer
 * Copyright (C) 2011 Tristan Matthews <le.businessman@gmail.com>
 * Copyright (C) 2010 Alessandro Decina <alessandro.d@gmail.com>
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
#include "gst-rtsp-cam-media-factory.h"

enum
{
  PROP_0,
  PROP_VIDEO,
  PROP_VIDEO_SOURCE,
  PROP_VIDEO_DEVICE,
  PROP_VIDEO_WIDTH,
  PROP_VIDEO_HEIGHT,
  PROP_VIDEO_BITRATE,
  PROP_VIDEO_FRAMERATE,
  PROP_VIDEO_CODEC,
  PROP_AUDIO,
  PROP_AUDIO_SOURCE,
  PROP_AUDIO_DEVICE,
  PROP_AUDIO_CODEC,
  PROP_AUDIO_CHANNELS
};

typedef struct
{
  gchar *name;
  gchar *encoder;
  gchar *encoder_settings;
  gchar *payloader;
} CodecDescriptor;

GST_DEBUG_CATEGORY_STATIC (rtsp_cam_media_factory_debug);
#define GST_CAT_DEFAULT rtsp_cam_media_factory_debug

static void gst_rtsp_cam_media_factory_get_property (GObject *object, guint propid,
    GValue *value, GParamSpec *pspec);
static void gst_rtsp_cam_media_factory_set_property (GObject *object, guint propid,
    const GValue *value, GParamSpec *pspec);
static void gst_rtsp_cam_media_factory_finalize (GObject * obj);
static GstElement * gst_rtsp_cam_media_factory_get_element (GstRTSPMediaFactory *factory,
    const GstRTSPUrl *url);
static gchar *gst_rtsp_cam_media_factory_gen_key (GstRTSPMediaFactory *factory, const GstRTSPUrl *url);

G_DEFINE_TYPE (GstRTSPCamMediaFactory, gst_rtsp_cam_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY)

#define DEFAULT_VIDEO TRUE
#define DEFAULT_VIDEO_SOURCE "v4l2src"
#define DEFAULT_VIDEO_DEVICE NULL
#define DEFAULT_VIDEO_WIDTH -1
#define DEFAULT_VIDEO_HEIGHT -1
#define DEFAULT_VIDEO_BITRATE -1
#define DEFAULT_VIDEO_FRAMERATE_N 0
#define DEFAULT_VIDEO_FRAMERATE_D 1
#define DEFAULT_VIDEO_CODEC "mpeg4"
#define DEFAULT_AUDIO TRUE
#define DEFAULT_AUDIO_SOURCE "autoaudiosrc"
#define DEFAULT_AUDIO_DEVICE NULL
#define DEFAULT_AUDIO_CODEC "raw"
#define DEFAULT_AUDIO_CHANNELS -1
static const int VIDEO_PAYLOAD_TYPE = 96;
static const int AUDIO_PAYLOAD_TYPE = 97;

/* TODO: this should be returned from a function, so that we can pre-configure
 properties for the codecs and payloaders. */
static CodecDescriptor codecs[] = {
  { "mpeg4", "ffenc_mpeg4", NULL, "rtpmp4vpay" },
  { "h263", "ffenc_h263p", NULL, "rtph263ppay" },
  { "theora", "theoraenc", NULL, "rtptheorapay" },
  { "h264", "x264enc", "tune=zerolatency vbv-buf-capacity=300 pass=qual intra-refresh=true", "rtph264pay" },
  { "raw", "identity", NULL, "rtpL16pay" },
  { "vorbis", "vorbisenc", NULL, "rtpvorbispay" },
  { "celt", "celtenc", NULL, "rtpceltpay" },
  { NULL, NULL, NULL, NULL }
};

static void
gst_rtsp_cam_media_factory_class_init (GstRTSPCamMediaFactoryClass * klass)
{
  GObjectClass *gobject_class;
  GstRTSPMediaFactoryClass *media_factory_class = GST_RTSP_MEDIA_FACTORY_CLASS (klass);

  gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->get_property = gst_rtsp_cam_media_factory_get_property;
  gobject_class->set_property = gst_rtsp_cam_media_factory_set_property;
  gobject_class->finalize = gst_rtsp_cam_media_factory_finalize;

  media_factory_class->get_element = gst_rtsp_cam_media_factory_get_element;
  media_factory_class->gen_key = gst_rtsp_cam_media_factory_gen_key;

  g_object_class_install_property (gobject_class, PROP_VIDEO,
      g_param_spec_boolean ("video", "Video", "video",
          DEFAULT_VIDEO, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_VIDEO_SOURCE,
      g_param_spec_string ("video-source", "Video source", "video source",
          DEFAULT_VIDEO_SOURCE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_VIDEO_DEVICE,
      g_param_spec_string ("video-device", "Video device", "video device",
          DEFAULT_VIDEO_DEVICE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_VIDEO_CODEC,
      g_param_spec_string ("video-codec", "Video codec", "video codec",
          DEFAULT_VIDEO_CODEC, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_VIDEO_WIDTH,
      g_param_spec_int ("video-width", "Video width", "video width",
          -1, G_MAXINT32, DEFAULT_VIDEO_WIDTH, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_VIDEO_HEIGHT,
      g_param_spec_int ("video-height", "Video height", "video height",
          -1, G_MAXINT32, DEFAULT_VIDEO_HEIGHT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_VIDEO_BITRATE,
      g_param_spec_int ("video-bitrate", "Video bitrate", "video bitrate",
          -1, G_MAXINT32, DEFAULT_VIDEO_BITRATE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_VIDEO_FRAMERATE,
      gst_param_spec_fraction ("video-framerate", "Video framerate", "video framerate",
          0, 1, G_MAXINT, 1,
          DEFAULT_VIDEO_FRAMERATE_N, DEFAULT_VIDEO_FRAMERATE_D,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_AUDIO,
      g_param_spec_boolean ("audio", "Audio", "audio",
          DEFAULT_AUDIO, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_AUDIO_SOURCE,
      g_param_spec_string ("audio-source", "Audio source", "audio source",
          DEFAULT_AUDIO_SOURCE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_AUDIO_DEVICE,
      g_param_spec_string ("audio-device", "Audio device", "audio device",
          DEFAULT_AUDIO_DEVICE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_AUDIO_CODEC,
      g_param_spec_string ("audio-codec", "Audio codec", "audio codec",
          DEFAULT_AUDIO_CODEC, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class, PROP_AUDIO_CHANNELS,
      g_param_spec_int ("audio-channels", "Audio channels", "audio channels",
          -1, G_MAXINT32, DEFAULT_AUDIO_CHANNELS, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  GST_DEBUG_CATEGORY_INIT (rtsp_cam_media_factory_debug,
      "rtspcammediafactory", 0, "RTSP Cam Media Factory");
}

static void
gst_rtsp_cam_media_factory_init (GstRTSPCamMediaFactory * factory)
{
  /* multiple clients can connect to this server */
  gst_rtsp_media_factory_set_shared (GST_RTSP_MEDIA_FACTORY (factory),
      TRUE);
}

static void
gst_rtsp_cam_media_factory_finalize (GObject * obj)
{
  GstRTSPCamMediaFactory *factory = GST_RTSP_CAM_MEDIA_FACTORY (obj);

  g_free (factory->video_source);
  g_free (factory->video_device);
  g_free (factory->video_codec);
  g_free (factory->audio_source);
  g_free (factory->audio_device);
  g_free (factory->audio_codec);

  G_OBJECT_CLASS (gst_rtsp_cam_media_factory_parent_class)->finalize (obj);
}

static void
gst_rtsp_cam_media_factory_get_property (GObject *object, guint propid,
    GValue *value, GParamSpec *pspec)
{
  GstRTSPCamMediaFactory *factory = GST_RTSP_CAM_MEDIA_FACTORY (object);

  switch (propid) {
    case PROP_VIDEO:
      g_value_set_boolean (value, factory->video);
      break;
    case PROP_AUDIO:
      g_value_set_boolean (value, factory->audio);
      break;
    case PROP_VIDEO_SOURCE:
      g_value_set_string (value, factory->video_source);
      break;
    case PROP_VIDEO_DEVICE:
      g_value_set_string (value, factory->video_device);
      break;
    case PROP_VIDEO_WIDTH:
      g_value_set_int (value, factory->video_width);
      break;
    case PROP_VIDEO_HEIGHT:
      g_value_set_int (value, factory->video_height);
      break;
    case PROP_VIDEO_BITRATE:
      g_value_set_int (value, factory->video_bitrate);
      break;
    case PROP_VIDEO_FRAMERATE:
      gst_value_set_fraction (value, factory->fps_n, factory->fps_d);
      break;
    case PROP_VIDEO_CODEC:
      g_value_set_string (value, factory->video_codec);
      break;
    case PROP_AUDIO_SOURCE:
      g_value_set_string (value, factory->audio_source);
      break;
    case PROP_AUDIO_DEVICE:
      g_value_set_string (value, factory->audio_device);
      break;
    case PROP_AUDIO_CODEC:
      g_value_set_string (value, factory->audio_codec);
      break;
    case PROP_AUDIO_CHANNELS:
      g_value_set_int (value, factory->audio_channels);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propid, pspec);
  }
}

static void
gst_rtsp_cam_media_factory_set_property (GObject *object, guint propid,
    const GValue *value, GParamSpec *pspec)
{
  GstRTSPCamMediaFactory *factory = GST_RTSP_CAM_MEDIA_FACTORY (object);

  switch (propid) {
    case PROP_VIDEO:
      factory->video = g_value_get_boolean (value);
      break;
    case PROP_AUDIO:
      factory->audio = g_value_get_boolean (value);
      break;
    case PROP_VIDEO_SOURCE:
      g_free (factory->video_source);
      factory->video_source = g_value_dup_string (value);
      if (factory->video_source == NULL)
        factory->video_source = g_strdup (DEFAULT_VIDEO_SOURCE);
      break;
    case PROP_VIDEO_DEVICE:
      g_free (factory->video_device);
      factory->video_device = g_value_dup_string (value);
      if (factory->video_device == NULL)
        factory->video_device = g_strdup (DEFAULT_VIDEO_DEVICE);
      break;
    case PROP_VIDEO_WIDTH:
      factory->video_width = g_value_get_int (value);
      break;
    case PROP_VIDEO_HEIGHT:
      factory->video_height = g_value_get_int (value);
      break;
    case PROP_VIDEO_BITRATE:
      factory->video_bitrate = g_value_get_int (value);
      break;
    case PROP_VIDEO_FRAMERATE:
      factory->fps_n = gst_value_get_fraction_numerator (value);
      factory->fps_d = gst_value_get_fraction_denominator (value);
      break;
    case PROP_VIDEO_CODEC:
      g_free (factory->video_codec);
      factory->video_codec = g_value_dup_string (value);
      if (factory->video_codec == NULL)
        factory->video_codec = g_strdup (DEFAULT_VIDEO_CODEC);
      break;
    case PROP_AUDIO_SOURCE:
      g_free (factory->audio_source);
      factory->audio_source = g_value_dup_string (value);
      if (factory->audio_source == NULL)
        factory->audio_source = g_strdup (DEFAULT_AUDIO_SOURCE);
      break;
    case PROP_AUDIO_DEVICE:
      g_free (factory->audio_device);
      factory->audio_device = g_value_dup_string (value);
      if (factory->audio_device == NULL)
        factory->audio_device = g_strdup (DEFAULT_AUDIO_DEVICE);
      break;
    case PROP_AUDIO_CODEC:
      g_free (factory->audio_codec);
      factory->audio_codec = g_value_dup_string (value);
      if (factory->audio_codec == NULL)
        factory->audio_codec = g_strdup (DEFAULT_AUDIO_CODEC);
      break;
    case PROP_AUDIO_CHANNELS:
      factory->audio_channels = g_value_get_int (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, propid, pspec);
  }
}

GstRTSPCamMediaFactory * gst_rtsp_cam_media_factory_new ()
{
  GstRTSPCamMediaFactory *factory;

  factory = g_object_new (GST_TYPE_RTSP_CAM_MEDIA_FACTORY, NULL);

  return factory;
}

static CodecDescriptor *
find_codec (gchar *codec_name)
{
  int i;
  CodecDescriptor *codec = NULL;

  for (i = 0; codecs[i].payloader != NULL; i++) {
    codec = &codecs[i];

    if (!g_strcmp0 (codec->name, codec_name))
      return codec;
  }

  return NULL;
}

static void
set_encoder_settings (GstElement *encoder, gchar *settings)
{
    int i;
    gchar **params = g_strsplit_set(settings, "= ", -1);
    /* for each property=value pair, we set it on our encoder */
    for (i = 0; params[i] != NULL; i+=2) {
        gst_util_set_object_arg (G_OBJECT(encoder), params[i], params[i+1]);
    }
    g_strfreev(params);
}

static GstElement *
create_encoder (GstRTSPCamMediaFactory *factory,
    gchar *codec_name)
{
  CodecDescriptor *codec;
  GstElement *encoder;

  codec = find_codec (codec_name);
  if (codec == NULL) {
    GST_ERROR_OBJECT (factory, "invalid codec %s", codec_name);
    return NULL;
  }

  encoder = gst_element_factory_make(codec->encoder, NULL);
  if (codec->encoder_settings)
      set_encoder_settings(encoder, codec->encoder_settings);

  return encoder;
}

static GstElement *
create_payloader (GstRTSPCamMediaFactory *factory,
    gchar *codec_name, gint payloader_number, gint payload_type)
{
  CodecDescriptor *codec;
  GstElement *payloader;
  gchar *name;

  codec = find_codec (codec_name);
  if (codec == NULL) {
    GST_ERROR_OBJECT (factory, "invalid codec %s", codec_name);
    return NULL;
  }

  name = g_strdup_printf ("pay%d", payloader_number);
  payloader = gst_element_factory_make(codec->payloader, name);
  g_free (name);
  g_object_set(payloader, "pt", payload_type, NULL);

  return payloader;
}

static GstElement *
create_video_payloader (GstRTSPCamMediaFactory *factory,
    GstElement *bin, gint payloader_number)
{
  GstElement *encoder;
  GstElement *pay;
  GstElement *videosrc;
  GstElement *queue, *ffmpegcolorspace, *videoscale, *videorate;
  GstElement *capsfilter;
  gchar *image_formats[] = {"video/x-raw-rgb",
      "video/x-raw-yuv", "video/x-raw-gray", NULL};
  GstCaps *video_caps;
  gchar *capss;
  gboolean linked;
  int i;

  encoder = create_encoder (factory, factory->video_codec);
  if (encoder == NULL)
    return NULL;
  if (factory->video_bitrate != -1)
    g_object_set (encoder, "bitrate", factory->video_bitrate, NULL);

  pay = create_payloader (factory, factory->video_codec, payloader_number, VIDEO_PAYLOAD_TYPE);
  if (pay == NULL)
    return NULL;

  videosrc = gst_element_factory_make (factory->video_source, NULL);
  if (g_strcmp0(factory->video_source, "videotestsrc") == 0)
      g_object_set (videosrc, "is-live", TRUE, NULL);
  else if (factory->video_device) /* don't set device for testsrc */
    g_object_set (videosrc, "device", factory->video_device, NULL);

  queue = gst_element_factory_make ("queue", NULL);
  ffmpegcolorspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
  videorate = gst_element_factory_make ("videorate", NULL);
  videoscale = gst_element_factory_make ("videoscale", NULL);
  capsfilter = gst_element_factory_make ("capsfilter", NULL);

  video_caps = gst_caps_new_empty ();
  for (i = 0; image_formats[i] != NULL; i++) {
    GstStructure *structure = gst_structure_new (image_formats[i], NULL);

    if (factory->video_width != -1)
      gst_structure_set (structure, "width", G_TYPE_INT, factory->video_width, NULL);

    if (factory->video_height != -1)
      gst_structure_set (structure, "height", G_TYPE_INT, factory->video_height, NULL);

    if (factory->fps_n != 0 && factory->fps_d != 0)
      gst_structure_set (structure, "framerate", GST_TYPE_FRACTION,
          factory->fps_n, factory->fps_d, NULL);

    gst_caps_append_structure (video_caps, structure);
  }

  capss = gst_caps_to_string (video_caps);
  GST_INFO_OBJECT (factory, "setting video caps %s", capss);
  g_free (capss);

  g_object_set (capsfilter, "caps", video_caps, NULL);
  gst_caps_unref (video_caps);

  gst_bin_add_many (GST_BIN (bin), videosrc, queue, ffmpegcolorspace, videoscale,
      videorate, capsfilter, encoder, pay, NULL);
  linked = gst_element_link_many (videosrc, queue, videorate, ffmpegcolorspace, videoscale,
      capsfilter, encoder, pay, NULL);
  g_assert(linked);

  return pay;
}

static GstElement *
create_audio_payloader (GstRTSPCamMediaFactory *factory,
    GstElement *bin, gint payloader_number)
{
  GstElement *encoder;
  GstElement *pay;
  GstElement *audiosrc;
  GstElement *audioconvert;
  GstElement *audiorate;
  GstElement *capsfilter;
  gchar *audio_formats[] = {"audio/x-raw-float",
      "audio/x-raw-int", NULL};
  GstCaps *audio_caps;
  gchar *capss;
  gboolean linked;
  int i;

  encoder = create_encoder (factory, factory->audio_codec);
  if (encoder == NULL) {
    GST_WARNING_OBJECT (factory, "couldn't create encoder ");
    return NULL;
  }

  pay = create_payloader (factory, factory->audio_codec, payloader_number, AUDIO_PAYLOAD_TYPE);
  if (pay == NULL) {
    GST_WARNING_OBJECT (factory, "couldn't create payloader ");
    gst_object_unref (encoder);
    return NULL;
  }

  audiosrc = gst_element_factory_make(factory->audio_source, NULL);
  if (audiosrc == NULL) {
    GST_WARNING_OBJECT (factory, "couldn't create audio source");
    gst_object_unref (encoder);
    gst_object_unref (pay);

    return NULL;
  }
  else /* FIXME: this is only valid for jackaudiosrc, we should proxy these elements' properties */
      g_object_set(audiosrc, "connect", 2, NULL);

  audioconvert = gst_element_factory_make ("audioconvert", NULL);
  audiorate = gst_element_factory_make ("audiorate", NULL);
  capsfilter = gst_element_factory_make ("capsfilter", NULL);

  audio_caps = gst_caps_new_empty ();
  for (i = 0; audio_formats[i] != NULL; i++) {
    GstStructure *structure = gst_structure_new (audio_formats[i], NULL);

    if (factory->audio_channels != -1)
      gst_structure_set (structure, "channels", G_TYPE_INT, factory->audio_channels, NULL);

    gst_caps_append_structure (audio_caps, structure);
  }

  capss = gst_caps_to_string (audio_caps);
  GST_INFO_OBJECT (factory, "setting audio caps %s", capss);
  g_free (capss);

  g_object_set (capsfilter, "caps", audio_caps, NULL);
  gst_caps_unref (audio_caps);

  gst_bin_add_many (GST_BIN (bin), audiosrc, capsfilter, audioconvert, audiorate, encoder, pay, NULL);
  linked = gst_element_link_many (audiosrc, capsfilter, audioconvert, audiorate, encoder, pay, NULL);
  if (!linked) {
      gst_object_unref (bin);
      return NULL;
  }
  return pay;
}

static GstElement *
gst_rtsp_cam_media_factory_get_element (GstRTSPMediaFactory *media_factory,
    const GstRTSPUrl *url)
{
  GstElement *video_payloader = NULL;
  GstElement *audio_payloader = NULL;
  GstElement *bin = NULL;
  gint payloader_number = 0;
  GstRTSPCamMediaFactory *factory = GST_RTSP_CAM_MEDIA_FACTORY (media_factory);
  (void) url; /* unused */

  bin = gst_bin_new (NULL);

  if (factory->video) {
    video_payloader = create_video_payloader(factory, bin, payloader_number);
    if (video_payloader) {
      GST_INFO_OBJECT (factory, "created video payloader %s",
          gst_element_get_name (video_payloader));
      payloader_number += 1;
    }
  }

  if (factory->audio) {
    audio_payloader = create_audio_payloader(factory, bin, payloader_number);
    if (audio_payloader)
      GST_INFO_OBJECT (factory, "created audio payloader %s",
            gst_element_get_name (audio_payloader));
  }

  if (!video_payloader && !audio_payloader) {
    GST_ERROR_OBJECT (factory, "no audio and no video");

    gst_object_unref (bin);
    return NULL;
  }

  return bin;
}

static gchar *
gst_rtsp_cam_media_factory_gen_key (GstRTSPMediaFactory *factory, const GstRTSPUrl *url)
{
  (void) factory; /* unused */
  return g_strdup (url->abspath);
}

