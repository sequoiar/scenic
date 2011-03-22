/* milhouse.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful, * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cstdlib>
#include <iostream>

#include "util/logWriter.h"

#include "gtk_utils.h"

#include "gst/videoFactory.h"
#include "gst/audioFactory.h"
#include "gst/messageDispatcher.h"
#include "gst/pipeline.h"
#include "gst/rtpReceiver.h"
#include "gst/videoConfig.h"
#include "playback.h"

#include "milhouse.h"
#include "milhouseLogger.h"
#include "programOptions.h"
#include "rtsp/RTSPServer.h"
#include "rtsp/RTSPClient.h"

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

    Playback playback(pipeline);
    shared_ptr<VideoReceiver> vRx;
    shared_ptr<AudioReceiver> aRx;

    if (enableVideo)       
    {
        vRx = videofactory::buildVideoReceiver(pipeline, options);
    }
    if (enableAudio)
    {
        aRx = audiofactory::buildAudioReceiver(pipeline, options);

        if (options["disable-jack-autoconnect"].as<bool>())
            MessageDispatcher::sendMessage("disable-jack-autoconnect");
    }

    playback.start();

    /// These options are more like commands, they are dispatched after playback starts
    if (options.count("jitterbuffer"))
        RtpReceiver::setLatency(options["jitterbuffer"].as<int>());

    if (enableVideo)
    {
        if(options["fullscreen"].as<bool>())
            MessageDispatcher::sendMessage("fullscreen");
        MessageDispatcher::sendMessage("window-title", options["window-title"].as<string>());
    }

    LOG_DEBUG("Running main loop");
    gutil::runMainLoop(options["timeout"].as<int>());
    LOG_DEBUG("main loop has finished");

    playback.stop();
}


void Milhouse::runAsSender(const po::variables_map &options, bool enableVideo, bool enableAudio)
{
    using std::tr1::shared_ptr;

    LOG_DEBUG("Running as sender");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline.makeVerbose();

    Playback playback(pipeline);
    shared_ptr<VideoSender> vTx;
    shared_ptr<AudioSender> aTx;

    if (enableVideo)
    {
        vTx = videofactory::buildVideoSender(pipeline, options);
    }

    if (enableAudio)
    {
        aTx = audiofactory::buildAudioSender(pipeline, options);

        if (options["disable-jack-autoconnect"].as<bool>())
            MessageDispatcher::sendMessage("disable-jack-autoconnect");
    }

    playback.start();

    gutil::runMainLoop(options["timeout"].as<int>());

    playback.stop();
}


void Milhouse::runAsLocal(const po::variables_map &options, bool enableVideo, bool enableAudio)
{
    using std::tr1::shared_ptr;

    LOG_DEBUG("Running local");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline.makeVerbose();

    Playback playback(pipeline);
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
        if (options["disable-jack-autoconnect"].as<bool>())
            MessageDispatcher::sendMessage("disable-jack-autoconnect");
    }

    playback.start();

    if (enableVideo)
    {
        if(options["fullscreen"].as<bool>())
            MessageDispatcher::sendMessage("fullscreen");
        MessageDispatcher::sendMessage("window-title", options["window-title"].as<std::string>());
    }

    gutil::runMainLoop(options["timeout"].as<int>());

    playback.stop();
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

    if (options["dump-pipeline"].as<bool>())
        setenv("GST_DEBUG_DUMP_DOT_DIR", ".", 0 /* don't override current value if present*/);

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
        LOG_ERROR("argument error: must be sender OR receiver OR localvideo."); 
        return 1;
    }


    bool enableVideo = not options["disable-video"].as<bool>();
    bool enableAudio = not options["disable-audio"].as<bool>();
    enableVideo = enableVideo and options.count("videoport");
    enableAudio = enableAudio and options.count("audioport");

    if (not enableVideo and not enableAudio)
    {
        LOG_ERROR("argument error: must provide videoport and/or audioport. see --help");
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

