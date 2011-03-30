/* RTSPServer.cpp
 * Copyright (C) 2011 Société des arts technologiques (SAT)
 * Copyright (C) 2011 Tristan Matthews
 * http://www.sat.qc.ca
 * All rights reserved.
 *
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

#include "RTSPServer.h"
#include <cstring>
#include <gst/gst.h>
#pragma GCC diagnostic ignored "-pedantic"
#include <gst/rtsp-server/rtsp-server.h>

#include <boost/program_options.hpp>

#include "gst-rtsp-cam-media-factory.h"
#include "util/logWriter.h"
#include "gutil/gutil.h"

namespace {
static gboolean
timeout (GstRTSPServer *server)
{
  GstRTSPSessionPool *pool;

  pool = gst_rtsp_server_get_session_pool (server);
  gst_rtsp_session_pool_cleanup (pool);
  g_object_unref (pool);

  return TRUE;
}
}

RTSPServer::RTSPServer(const boost::program_options::variables_map &options)
{
  using std::string;
  GstRTSPServer *server;
  GstRTSPMediaMapping *mapping;
  GstRTSPCamMediaFactory *factory;
  GstRTSPUrl *local_url;
  string urlStr("rtsp://");
  string remoteHost(options["address"].as<string>());
  // FIXME: temporary workaround for https://bugzilla.gnome.org/show_bug.cgi?id=595840
  if (remoteHost == "localhost")
      remoteHost = "127.0.0.1";
  urlStr += remoteHost;
  urlStr += ":8554/milhouse";

  if (gst_rtsp_url_parse (urlStr.c_str(), &local_url) != GST_RTSP_OK)
      THROW_ERROR("Invalid uri " << urlStr);

  bool enableVideo = not options["disable-video"].as<bool>();
  bool enableAudio = not options["disable-audio"].as<bool>();
  if (enableVideo)
      LOG_DEBUG("Video enabled");
  if (enableAudio)
      LOG_DEBUG("Audio enabled");

  server = gst_rtsp_server_new ();

  factory = gst_rtsp_cam_media_factory_new ();
  g_object_set (factory,
          "video", enableVideo,
          "video-source", options["videosource"].as<string>().c_str(),
          "video-device", options["videodevice"].as<string>().c_str(),
          "video-width", options["width"].as<int>(),
          "video-height", options["height"].as<int>(),
          "video-bitrate", options["videobitrate"].as<int>(),
          "video-codec", options["videocodec"].as<string>().c_str(),
          "video-framerate", options["framerate"].as<int>(), 1,
          "audio", enableAudio,
          "audio-source", options["audiosource"].as<string>().c_str(),
          "audio-device", options["audiodevice"].as<string>().c_str(),
          "audio-codec", options["audiocodec"].as<string>().c_str(),
          "audio-channels", options["numchannels"].as<int>(),
          NULL);

  mapping = gst_rtsp_server_get_media_mapping (server);
  gst_rtsp_media_mapping_add_factory (mapping, local_url->abspath,
          GST_RTSP_MEDIA_FACTORY (factory));
  g_object_unref (mapping);

  gst_rtsp_url_free (local_url);

  gst_rtsp_server_attach (server, NULL);

  g_timeout_add_seconds (5, (GSourceFunc) timeout, server);
}

void RTSPServer::run(int timeout)
{
    /* start main loop */
    gutil::runMainLoop(timeout);
}

