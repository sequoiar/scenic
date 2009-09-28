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

#include "milhouseLogger.h"


namespace pof 
{
    short run(int argc, char **argv);
}

int telnetServer(int, int);


void addOptions(OptionArgs &options)
{
    options.addBool("receiver", 'r', "receiver");
    options.addBool("sender", 's', "sender");
    options.addString("address", 'i', "address", "provide ip address of remote host");
    options.addString("videocodec", 'v', "videocodec", "h264");
    options.addString("audiocodec", 'a', "audiocodec", "vorbis raw mp3");
    options.addString("videosink", 'k', "videosink", "xvimagesink glimagesink");
    options.addString("audiosink", 'l', "audiosink", "jackaudiosink alsasink pulsesink");
    options.addInt("audioport", 't', "audioport", "portnum");
    options.addInt("videoport", 'p', "videoport", "portnum");
    options.addBool("fullscreen", 'f', "default to fullscreen");
    options.addString("shared-video-id", 'B', "shared video buffer id", "shared_memory");
    options.addBool("deinterlace", 'o', "deinterlace video");
    options.addString("videodevice", 'd', "device", "/dev/video0 /dev/video1");
    options.addString("audiodevice", 'q', "audio device", "hw:0 hw:2 plughw:0 plughw:2");
    options.addString("videolocation", 0, "video file location", "<filename>");
    options.addString("audiolocation", 0, "audio file location", "<filename>");
    options.addInt("screen", 'n', "screen", "xinerama screen num");
    options.addBool("version", 'w', "version number");
    options.addInt("numchannels", 'c', "numchannels", "2");
    options.addInt("videobitrate", 'x', "videobitrate", "3000000");
    options.addString("audiosource", 'e', "audiosource", "jackaudiosrc alsasrc pulsesrc");
    options.addString("videosource", 'u', "videosource", "v4l2src v4lsrc dv1394src");
    options.addInt("timeout", 'z', "timeout", "time in ms to wait before quitting, 0 means run indefinitely");
    options.addInt("audio-buffer-usec", 'b', "audiobuffer", "length of receiver's audio buffer in microseconds, must be > 10000");
    options.addInt("jitterbuffer", 'g', "jitterbuffer", "length of receiver's rtp jitterbuffers in milliseconds, must be > 1");
    options.addInt("camera-number", 'G', "camera_number", "camera id for dc1394");
    options.addString("multicast-interface", 'I', "multicast_interface", "interface to use for multicast");
    options.addBool("enable-controls", 'j', "enable gui controls for jitter buffer");
    options.addBool("disable-jack-autoconnect", 'J', "disable jack's autoconnection");
    options.addBool("caps-out-of-band", 'C', "send/receive caps out of band, default=false");
    //telnetServer param
    options.addInt("serverport", 'y', "run as server", "port to listen on");
}


short pof::run(int argc, char **argv)
{
    using boost::shared_ptr;

    OptionArgs options;
    addOptions(options);

    options.parse(argc, argv);

#ifdef SVNVERSION
    std::cout << "Ver:" << PACKAGE_VERSION << " Rev #" << SVNVERSION << std::endl;
#else
    std::cout << "Ver:" << PACKAGE_VERSION << std::endl;
#endif
    if (argc == 1)  // we printed help msg in parse, no need to continue
        return 0;

    if(options["serverport"])
        return telnetServer(options["sender"], options["serverport"]);

    MilhouseLogger logger; // just instantiate, his base class will know what to do 

    LOG_INFO("Built on " << __DATE__ << " at " << __TIME__);

    if(options["version"])
    {
#ifdef SVNVERSION
            LOG_INFO("version " << PACKAGE_VERSION <<  " Svn Revision: " << SVNVERSION << std::endl);
#else
            LOG_INFO("version " << PACKAGE_VERSION << std::endl);
#endif
        return 0;
    }

    if ((!options["sender"] and !options["receiver"]) or (options["sender"] and options["receiver"]))
        THROW_CRITICAL("argument error: must be sender OR receiver. see --help"); 

    if (options["enable-controls"])
    {
        if (options["receiver"] )
            RtpReceiver::enableControl();
        else if (options["sender"])   // sender
            RtpSender::enableControl();
    }

    // Must have these arguments
    int disableVideo = !options["videocodec"] and !options["videoport"];
    int disableAudio = !options["audiocodec"] and !options["audioport"];

    if (disableVideo)
        LOG_DEBUG("Video disabled.");
    if (disableAudio) 
        LOG_DEBUG("Audio disabled.");

    if (disableVideo and disableAudio)
        THROW_CRITICAL("argument error: must provide video and/or audio parameters. see --help");

    if (options["videoport"] == options["audioport"])
        THROW_CRITICAL("Videoport and audioport cannot be equal"); // Fail early, other port checks do happen later too

    if (options["receiver"]) 
    {
        LOG_DEBUG("Running as receiver");
        shared_ptr<VideoReceiver> vRx;
        shared_ptr<AudioReceiver> aRx;


        if (!disableVideo)       
        {
            MapMsg ipcp(videofactory::rxOptionsToIPCP(options));
            vRx = videofactory::buildVideoReceiver(ipcp);
        }
        if (!disableAudio)
        {
            MapMsg ipcp(audiofactory::rxOptionsToIPCP(options));
            aRx = audiofactory::buildAudioReceiver(ipcp);

            if (options["disable-jack-autoconnect"])
                MessageDispatcher::sendMessage("disable-jack-autoconnect");
        }

#ifdef CONFIG_DEBUG_LOCAL
        //playback::makeVerbose();
#endif

        playback::start();

        /// These options are more like commands, they are dispatched after playback starts
        if (options["jitterbuffer"])
            RtpReceiver::setLatency(options["jitterbuffer"]);

        if (!disableVideo)
        {
            if(options["fullscreen"])
                MessageDispatcher::sendMessage("fullscreen");
        }

        if (!options["timeout"]) // run for infinite amount of time
            options["timeout"] = 0;

        gutil::runMainLoop(options["timeout"]);

        tassert(playback::isPlaying() or playback::quitted());

        playback::stop();
    }
    else 
    {
        LOG_DEBUG("Running as sender");
        shared_ptr<VideoSender> vTx;
        shared_ptr<AudioSender> aTx;

        if (!disableVideo)
        {
            MapMsg ipcp(videofactory::txOptionsToIPCP(options));
            vTx = videofactory::buildVideoSender(ipcp);
        }

        if (!disableAudio)
        {
            MapMsg ipcp(audiofactory::txOptionsToIPCP(options));
            aTx = audiofactory::buildAudioSender(ipcp);

            if (options["disable-jack-autoconnect"])
                MessageDispatcher::sendMessage("disable-jack-autoconnect");
        }

#ifdef CONFIG_DEBUG_LOCAL
        //playback::makeVerbose();
#endif

        playback::start();

        if (!options["timeout"]) // run for finite amount of time
            options["timeout"] = 0;

        gutil::runMainLoop(options["timeout"]);

        tassert(playback::isPlaying() or playback::quitted());

        playback::stop();
    }
    return 0;
}

void onExit()
{
    std::cout << "bye." << std::endl;
}


int main(int argc, char **argv)
{
    int ret = 0;
    atexit (onExit);
    try {
        set_handler();
        ret = pof::run(argc, argv);
    }
    catch (std::exception e)
    {
        std::cout << "Main Thread LEAVING with exception: " << e.what() << std::endl;
        ret = 1;
    }
    return ret;
}

