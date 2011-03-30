/* milhouse.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
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
 * Scenic is distributed in the hope that it will be useful, * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdlib>
#include <iostream>

#include "milhouse.h"
#include "util/log_writer.h"

#include "gutil/gutil.h"

#include "gst/video_factory.h"
#include "gst/audio_factory.h"
#include "gst/pipeline.h"
#include "gst/rtp_receiver.h"
#include "gst/video_config.h"

#include "milhouse_logger.h"
#include "program_options.h"
#include "rtsp/rtsp_server.h"
#include "rtsp/rtsp_client.h"

namespace po = boost::program_options;
void Milhouse::runAsRTSPClient(const po::variables_map &options)
{
    LOG_DEBUG("Running as RTSP client");
    RTSPClient client(options);
    client.run(options["timeout"].as<int>());
}

void Milhouse::runAsRTSPServer(const po::variables_map &options)
{
    LOG_DEBUG("Running as RTSP server");
    // TODO: create a server that uses rtsp-cam-media-factory, see rtsp/examples/gst-rtsp-cam.c
    RTSPServer server(options);
    server.run(options["timeout"].as<int>());
}

void Milhouse::runAsReceiver(const po::variables_map &options, bool enableVideo, bool enableAudio)
{
    using std::tr1::shared_ptr;
    using std::string;

    LOG_DEBUG("Running as receiver");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<string>() == "gst-debug")
        pipeline.makeVerbose();

    shared_ptr<AudioReceiver> aRx;
    shared_ptr<VideoReceiver> vRx;

    if (enableAudio)
        aRx = audiofactory::buildAudioReceiver(pipeline, options);

    if (enableVideo)
        vRx = videofactory::buildVideoReceiver(pipeline, options);

    pipeline.start();

    /// These options are more like commands, they are dispatched after playback starts
    if (options.count("jitterbuffer"))
        RtpReceiver::setLatency(options["jitterbuffer"].as<int>());

    LOG_DEBUG("Running main loop");
    gutil::runMainLoop(options["timeout"].as<int>());
    LOG_DEBUG("main loop has finished");

    pipeline.stop();
}


void Milhouse::runAsSender(const po::variables_map &options, bool enableVideo, bool enableAudio)
{
    using std::tr1::shared_ptr;

    LOG_DEBUG("Running as sender");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline.makeVerbose();

    shared_ptr<AudioSender> aTx;
    shared_ptr<VideoSender> vTx;

    if (enableAudio)
        aTx = audiofactory::buildAudioSender(pipeline, options);

    if (enableVideo)
        vTx = videofactory::buildVideoSender(pipeline, options);

    pipeline.start();

    gutil::runMainLoop(options["timeout"].as<int>());

    pipeline.stop();
}


void Milhouse::runAsLocal(const po::variables_map &options, bool enableVideo, bool enableAudio)
{
    using std::tr1::shared_ptr;

    LOG_DEBUG("Running local");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline.makeVerbose();

    shared_ptr<LocalVideo> localVideo;
    if (enableVideo)
    {
        LOG_DEBUG("LOCAL VIDEO");
        localVideo = videofactory::buildLocalVideo(pipeline, options);
    }

    shared_ptr<LocalAudio> localAudio;
    if (enableAudio)
    {
        LOG_DEBUG("LOCAL AUDIO");
        localAudio = audiofactory::buildLocalAudio(pipeline, options);
    }

    pipeline.start();

    gutil::runMainLoop(options["timeout"].as<int>());

    pipeline.stop();
}


short Milhouse::usage(const po::options_description &desc)
{
    std::cout << desc << "\n";
    return 0;
}


short Milhouse::run(int argc, char **argv)
{
    using std::string;
    po::options_description desc(ProgramOptions::createDefaultOptions());
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);

    if (options.count("help") or argc == 1)
        return usage(desc);

    MilhouseLogger logger(options["debug"].as<string>()); // just instantiate, his base class will know what to do

    LOG_DEBUG("Built on " << __DATE__ << " at " << __TIME__);

    if (options["version"].as<bool>())
    {
        LOG_PRINT("milhouse version " << PACKAGE_VERSION << std::endl);
        return 0;
    }

    // maybe just have a separate function that checks for all these standalone
    // calls and quits instead of all here
    if (options["list-v4l2"].as<bool>())
        return VideoSourceConfig::listV4lDevices();
    else if (options["list-cameras"].as<bool>())
        return VideoSourceConfig::listCameras();
    else if (options.count("v4l2-standard"))
    {
        VideoSourceConfig::setStandard(options["videodevice"].as<string>(), options["v4l2-standard"].as<string>());
        return 0;
    }
    else if (options.count("v4l2-input"))
    {
        VideoSourceConfig::setInput(options["videodevice"].as<string>(), options["v4l2-input"].as<int>());
        return 0;
    }

    // wrapper so main doesn't need to know about gst and gtk
    gutil::init_gst_gtk(argc, argv);

    if (options["gst-version"].as<bool>())
    {
        // this was handled internally by gst_init's argv parsing
        return 0;
    }

    if (options["max-channels"].as<bool>())
    {
        audiofactory::printMaxChannels(options["audiocodec"].as<string>());
        return 0;
    }

    /*----------------------------------------------*/
    // RTSP mode
    /*----------------------------------------------*/

    if (options["rtsp-server"].as<bool>())
    {
        runAsRTSPServer(options);
        return 0;
    }
    else if (options["rtsp-client"].as<bool>())
    {
        runAsRTSPClient(options);
        return 0;
    }

    /*----------------------------------------------*/
    // Local preview mode
    /*----------------------------------------------*/

    bool enableLocalVideo = options["localvideo"].as<bool>();
    bool enableLocalAudio = options["localaudio"].as<bool>();
    if (enableLocalVideo or enableLocalAudio)
    {
        LOG_DEBUG("Running as local preview");
        runAsLocal(options, enableLocalVideo, enableLocalAudio);
        return 0;
    }

    /*----------------------------------------------*/
    // Send/receive mode
    /*----------------------------------------------*/

    if ((not options["sender"].as<bool>() and not options["receiver"].as<bool>())
            or (options["sender"].as<bool>() and options["receiver"].as<bool>()))
    {
        LOG_ERROR("Must be run as "
                "-s, -r, --localvideo, --localaudio, --rtsp-server OR --rtsp-client."
                " See --help.");
        return 1;
    }


    bool enableVideo = not options["disable-video"].as<bool>();
    bool enableAudio = not options["disable-audio"].as<bool>();

    if (not enableVideo and not enableAudio)
    {
        LOG_ERROR("argument error: cannot disable video and audio. see --help");
        return 1;
    }

    // Fail early, other port checks do happen later too
    if (enableAudio and enableVideo)
        if (options["videoport"].as<int>() == options["audioport"].as<int>())
        {
            LOG_ERROR("argument error: videoport and audioport cannot be equal");
            return 1;
        }

    if (options["receiver"].as<bool>())
        runAsReceiver(options, enableVideo, enableAudio);
    else
        runAsSender(options, enableVideo, enableAudio);

    return 0;
}

