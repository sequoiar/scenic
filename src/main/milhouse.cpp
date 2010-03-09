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

#include "util.h"

#include "gutil.h"

#include "gst/videoFactory.h"
#include "gst/audioFactory.h"
#include "gst/messageDispatcher.h"
#include "gst/pipeline.h"
#include "playback.h"

#include "milhouse.h"
#include "milhouseLogger.h"
#include "programOptions.h"

namespace po = boost::program_options;

void Milhouse::runAsReceiver(const po::variables_map &options, bool disableVideo, bool disableAudio)
{
    using boost::shared_ptr;

    LOG_DEBUG("Running as receiver");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline.makeVerbose();

    Playback playback(pipeline);
    shared_ptr<VideoReceiver> vRx;
    shared_ptr<AudioReceiver> aRx;

    if (not disableVideo)       
    {
        vRx = videofactory::buildVideoReceiver(pipeline, options);
    }
    if (not disableAudio)
    {
        aRx = audiofactory::buildAudioReceiver(pipeline, options);

        if (options["disable-jack-autoconnect"].as<bool>())
            MessageDispatcher::sendMessage("disable-jack-autoconnect");
    }

    playback.start();

    /// These options are more like commands, they are dispatched after playback starts
    if (options.count("jitterbuffer"))
        RtpReceiver::setLatency(options["jitterbuffer"].as<int>());

    if (!disableVideo)
    {
        if(options["fullscreen"].as<bool>())
            MessageDispatcher::sendMessage("fullscreen");
        MessageDispatcher::sendMessage("window-title", options["window-title"].as<std::string>());
    }

    LOG_DEBUG("Running main loop");
    gutil::runMainLoop(options["timeout"].as<int>());
    LOG_DEBUG("main loop has finished");

    playback.stop();
}


void Milhouse::runAsSender(const po::variables_map &options, bool disableVideo, bool disableAudio)
{
    using boost::shared_ptr;

    LOG_DEBUG("Running as sender");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline.makeVerbose();

    Playback playback(pipeline);
    shared_ptr<VideoSender> vTx;
    shared_ptr<AudioSender> aTx;

    if (!disableVideo)
    {
        vTx = videofactory::buildVideoSender(pipeline, options);
    }

    if (!disableAudio)
    {
        aTx = audiofactory::buildAudioSender(pipeline, options);

        if (options["disable-jack-autoconnect"].as<bool>())
            MessageDispatcher::sendMessage("disable-jack-autoconnect");
    }

    playback.start();

    gutil::runMainLoop(options["timeout"].as<int>());

    playback.stop();
}


void Milhouse::runAsLocal(const po::variables_map &options)
{
    using boost::shared_ptr;

    LOG_DEBUG("Running local");
    Pipeline pipeline; // Pipeline will go out of scope last
    if (options["debug"].as<std::string>() == "gst-debug")
        pipeline.makeVerbose();

    Playback playback(pipeline);
    shared_ptr<LocalVideo> localVideo;
    //shared_ptr<LocalAudio> localAudio; // FIXME: doesn't exist (yet)

    localVideo = videofactory::buildLocalVideo(pipeline, options);

    playback.start();
    
    if(options["fullscreen"].as<bool>())
        MessageDispatcher::sendMessage("fullscreen");
    MessageDispatcher::sendMessage("window-title", options["window-title"].as<std::string>());

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
    po::options_description desc(ProgramOptions::createDefaultOptions());
    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);

#ifdef SVNVERSION
    std::cout << "Ver:" << PACKAGE_VERSION << " Rev #" << SVNVERSION << std::endl;
#else
    std::cout << "Ver:" << PACKAGE_VERSION << std::endl;
#endif

    if (options.count("help") or argc == 1) 
        return usage(desc);

    MilhouseLogger logger(options["debug"].as<std::string>()); // just instantiate, his base class will know what to do 

    LOG_INFO("Built on " << __DATE__ << " at " << __TIME__);

    if (options["version"].as<bool>())
    {
#ifdef SVNVERSION
        LOG_INFO("version " << PACKAGE_VERSION <<  " Svn Revision: " << SVNVERSION << std::endl);
#else
        LOG_INFO("version " << PACKAGE_VERSION << std::endl);
#endif
        return 0;
    }

    if (options.count("display"))
    {
        setenv("DISPLAY", 
                options["display"].as<std::string>().c_str(), 
                1 /* override current value if present */);
    }

    if (options["list-cameras"].as<bool>())
        return VideoSourceConfig::listCameras();

    if (options.count("v4l2-standard"))
    {
        VideoSourceConfig::setStandard(options["videodevice"].as<std::string>(), options["v4l2-standard"].as<std::string>());
        return 0;
    }
    
    if (options.count("v4l2-input"))
    {
        VideoSourceConfig::setInput(options["videodevice"].as<std::string>(), options["v4l2-input"].as<int>());
        return 0;
    }

    if (options["localvideo"].as<bool>()) 
    {
        runAsLocal(options);
        return 0;
    }

    if ((!options["sender"].as<bool>() and !options["receiver"].as<bool>()) 
            or (options["sender"].as<bool>() and options["receiver"].as<bool>()))
    {
        LOG_ERROR("argument error: must be sender OR receiver OR localvideo."); 
        return 1;
    }

    bool disableVideo = !options.count("videoport");
    bool disableAudio = !options.count("audioport");


    if (disableVideo and disableAudio)
    {
        LOG_ERROR("argument error: must provide videoport and/or audioport. see --help");
        return 1;
    }

    // Fail early, other port checks do happen later too
    if (!disableAudio and !disableVideo)
        if (options["videoport"].as<int>() == options["audioport"].as<int>())
        {
            LOG_ERROR("argument error: videoport and audioport cannot be equal"); 
            return 1;
        }

    if (options["receiver"].as<bool>()) 
        runAsReceiver(options, disableVideo, disableAudio);
    else 
        runAsSender(options, disableVideo, disableAudio);

    return 0;
}

int main(int argc, char **argv)
{
    int ret = 0;

    try 
    {
        signal_handlers::setHandlers();
        Milhouse milhouse;
        ret = milhouse.run(argc, argv);
    }
    catch (const Except &e) // these are our exceptions, so we can assume they've already been logged
    {
        if (std::string(e.what()).find("INTERRUPTED") != std::string::npos)
        {
            std::cout << "Interrupted" << std::endl;
            ret = 0;
        }
        else
        {
#ifdef CONFIG_DEBUG_LOCAL
            std::cerr << "exitting with error: " << e.what() << std::endl;
#endif
            ret = 1;
        }
    }
    catch (const std::exception &e) // these are other exceptions (not one of our exception classes)
    {
        std::cerr << "exitting with error: " << e.what() << std::endl;
        ret = 1;
    }
    std::cout << "Exitting Milhouse" << std::endl;
    return ret;
}

