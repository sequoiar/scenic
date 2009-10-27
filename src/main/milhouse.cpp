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

#include "util.h"

#include "gutil.h"
#include "msgThreadFactory.h"

#define __COMMAND_LINE__
#include "gst/videoFactory.h"
#include "gst/audioFactory.h"
#undef __COMMAND_LINE__

#include "milhouse.h"
#include "milhouseLogger.h"
#include "programOptions.h"


int telnetServer(int, int);

namespace po = boost::program_options;

void Milhouse::runAsReceiver(const po::variables_map &options, bool disableVideo, bool disableAudio)
{
    using boost::shared_ptr;

    LOG_DEBUG("Running as receiver");
    shared_ptr<VideoReceiver> vRx;
    shared_ptr<AudioReceiver> aRx;

    if (options["enable-controls"].as<bool>())
        RtpReceiver::enableControl();

    if (!disableVideo)       
    {
        MapMsg ipcp(ProgramOptions::toMapMsg(options));
        videofactory::rxOptionsToIPCP(ipcp);
        vRx = videofactory::buildVideoReceiver(ipcp);
    }
    if (!disableAudio)
    {
        MapMsg ipcp(ProgramOptions::toMapMsg(options));
        audiofactory::rxOptionsToIPCP(ipcp);
        aRx = audiofactory::buildAudioReceiver(ipcp);

        if (options["disable-jack-autoconnect"].as<bool>())
            MessageDispatcher::sendMessage("disable-jack-autoconnect");
    }

    playback::start();

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

    playback::stop();
}


void Milhouse::runAsSender(const po::variables_map &options, bool disableVideo, bool disableAudio)
{
    using boost::shared_ptr;

    LOG_DEBUG("Running as sender");
    shared_ptr<VideoSender> vTx;
    shared_ptr<AudioSender> aTx;

    if (options["enable-controls"].as<bool>())
        RtpSender::enableControl();

    if (!disableVideo)
    {
        MapMsg ipcp(ProgramOptions::toMapMsg(options));
        videofactory::txOptionsToIPCP(ipcp);
        vTx = videofactory::buildVideoSender(ipcp);
    }

    if (!disableAudio)
    {
        MapMsg ipcp(ProgramOptions::toMapMsg(options));
        audiofactory::txOptionsToIPCP(ipcp);
        aTx = audiofactory::buildAudioSender(ipcp);

        if (options["disable-jack-autoconnect"].as<bool>())
            MessageDispatcher::sendMessage("disable-jack-autoconnect");
    }

    playback::start();

    gutil::runMainLoop(options["timeout"].as<int>());

    playback::stop();
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

    if (options.count("serverport"))
        return telnetServer(options["sender"].as<bool>(), options["serverport"].as<int>());

    MilhouseLogger logger(options["debug"].as<std::string>()); // just instantiate, his base class will know what to do 

    // FIXME: this is actually where pipeline instance is created because it's the 
    // first time we call Pipeline::Instance(), this is bad in its implicitness
    if (logger.gstDebug())
        playback::makeVerbose();

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

 
    if ((!options["sender"].as<bool>() and !options["receiver"].as<bool>()) 
            or (options["sender"].as<bool>() and options["receiver"].as<bool>()))
    {
        LOG_ERROR("argument error: must be sender OR receiver."); 
        return 1;
    }

    bool disableVideo = !options.count("videocodec") and !options.count("videoport");
    bool disableAudio = !options.count("audiocodec") and !options.count("audioport");


    if (disableVideo and disableAudio)
    {
        LOG_ERROR("argument error: must provide video and/or audio parameters. see --help");
        return 1;
    }

    if (disableVideo)
    {
        LOG_DEBUG("Video disabled.");
        if (!(options.count("audioport") and options.count("audiocodec")))
        {
            LOG_ERROR("argument error: must specify both audioport and audiocodec");
            return 1;
        }
    }
    if (disableAudio) 
    {
        LOG_DEBUG("Audio disabled.");
        if (!(options.count("videoport") and options.count("videocodec")))
        {
            LOG_ERROR("argument error: must specify both videoport and videocodec");
            return 1;
        }
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

void onExit()
{
#ifdef CONFIG_DEBUG_LOCAL
    std::cerr << "Leaving Milhouse" << std::endl;
#endif
}


int main(int argc, char **argv)
{
    int ret = 0;
    atexit(onExit);

    try {
        signal_handlers::setHandlers();
        Milhouse milhouse;
        ret = milhouse.run(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cout << "exitting with error: " << e.what() << std::endl;
        ret = 1;
    }
    return ret;
}

