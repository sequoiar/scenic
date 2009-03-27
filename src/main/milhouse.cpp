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
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include "gutil.h"
#include "msgThreadFactory.h"

#define USE_SMART_PTR //Factories return a shared_ptr 
#include "gst/videoFactory.h"
#include "gst/audioFactory.h"

#define BLOCK() gutil::runMainLoop(0); 

namespace pof 
{
    short run(int argc, char **argv);
    const short NUM_CHANNELS = 2;

#ifdef HAVE_BOOST
    using namespace boost;
#else
    using namespace std::tr1;
#endif
}

int telnetServer(int, int);

// 2way audio and video
short pof::run(int argc, char **argv)
{
    OptionArgs options;

    // add options here
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
    options.addBool("deinterlace", 'o', "deinterlace video");
    options.addString("videodevice", 'd', "device", "/dev/video0 /dev/video1");
    options.addString("audiodevice", 'q', "audio device", "hw:0 hw:2 plughw:0 plughw:2 filename");
    options.addInt("screen", 'n', "screen", "xinerama screen num");
    options.addBool("version", 'w', "version number");
    options.addInt("numchannels", 'c', "numchannels", "2");
    options.addInt("videobitrate", 'x', "videobitrate", "3000000");
    options.addString("audiosource", 'e', "audiosource", "jackaudiosrc alsasrc pulsesrc");
    options.addString("videosource", 'u', "videosource", "v4l2src v4lsrc dv1394src");

    //telnetServer param
    options.addInt("serverport", 'y', "run as server", "port to listen on");

    options.parse(argc, argv);

    if(options["version"])
    {
        LOG_INFO("version " << PACKAGE_VERSION << '\b' << RELEASE_CANDIDATE);
        return 0;
    }

    if ((!options["sender"] and !options["receiver"]) or (options["sender"] and options["receiver"]))
        THROW_ERROR("argument error: must be sender OR receiver. see --help"); 

    if(options["serverport"])
        return telnetServer(options["sender"], options["serverport"]);

    int disableVideo = !options["videocodec"] and !options["videoport"];
    int disableAudio = !options["audiocodec"] and !options["audioport"];

    if (disableVideo)
        LOG_DEBUG("Video disabled.");
    if (disableAudio) 
        LOG_DEBUG("Audio disabled.");

    if (disableVideo and disableAudio)
        THROW_ERROR("argument error: must provide video and/or audio parameters. see --help");

    if(!options["address"]) 
        THROW_ERROR("argument error: missing address. see --help");

    if(!disableVideo and !options["videocodec"])
        THROW_ERROR("argument error: missing videocodec. see --help");
    if(!disableVideo and !options["videoport"])
        THROW_ERROR("argument error: missing videoport. see --help");

    if(!disableAudio and !options["audiocodec"])
        THROW_ERROR("argument error: missing audiocodec. see --help");
    if(!disableAudio and !options["audioport"])
        THROW_ERROR("argument error: missing audioport. see --help");

    if (options["receiver"]) 
    {
        LOG_DEBUG("Running as receiver");
        shared_ptr<VideoReceiver> vRx;
        shared_ptr<AudioReceiver> aRx;

        if (!disableVideo)       
        {
            if(!options["videosink"])
                THROW_ERROR("argument error: missing videosink. see --help");

            vRx = videofactory::buildVideoReceiver(options["address"], options["videocodec"], 
                    options["videoport"], options["screen"], options["videosink"]);
        }
        if (!disableAudio)
        {
            if(!options["audiosink"])
                THROW_ERROR("argument error: missing audiosink. see --help");
            
            // FIXME: we should distinguish between device and location
            std::string audioLocation = "";
            if (options["audiodevice"])
                audioLocation = (std::string) options["audiodevice"];

            aRx = audiofactory::buildAudioReceiver(options["address"], options["audiocodec"], 
                    options["audioport"], options["audiosink"], audioLocation, audiofactory::AUDIO_BUFFER_USEC);
        }

#ifdef CONFIG_DEBUG_LOCAL
        playback::makeVerbose();
#endif

        playback::start();

        if (!disableVideo)
        {
            if(options["fullscreen"])
                vRx->makeFullscreen();
        }

        BLOCK();
        assert(playback::isPlaying() or playback::quitted());

        playback::stop();
    }
    else 
    {
        LOG_DEBUG("Running as sender");
        shared_ptr<VideoSender> vTx;
        shared_ptr<AudioSender> aTx;

        if (!disableVideo)
        {
            if (!options["videosource"])
                THROW_ERROR("argument error: missing --videosource. see --help");

            std::string videoDevice = ""; 

            if (options["videodevice"]) 
                videoDevice = (std::string) options["videodevice"]; 

            int videoBitrate = 3000000;
            if (options["videobitrate"]) 
                videoBitrate = options["videobitrate"];
            LOG_DEBUG("VIDEOBITRATE IS " << videoBitrate);

            VideoSourceConfig vConfig(options["videosource"], videoBitrate, 
                    videoDevice, options["deinterlace"]);

            vTx = videofactory::buildVideoSender(vConfig, options["address"], options["videocodec"], 
                    options["videoport"]);
        }

        if (!disableAudio)
        {
            if (!options["audiosource"])
                THROW_ERROR("argument error: missing --audiosource. see --help");

            int numChannels = 2;
            if (options["numchannels"]) 
                numChannels = options["numchannels"];

            std::string audioLocation = "";
            if (options["audiodevice"])
                audioLocation = (std::string) options["audiodevice"];

            AudioSourceConfig aConfig(options["audiosource"], audioLocation, numChannels);
            aTx = audiofactory::buildAudioSender(aConfig, options["address"], options["audiocodec"], options["audioport"]);
        }

#ifdef CONFIG_DEBUG_LOCAL
        playback::makeVerbose();
#endif

        playback::start();

        if (!disableVideo)
            assert(tcpSendBuffer(options["address"], ports::CAPS_OFFSET + (int)options["videoport"], videofactory::MSG_ID, vTx->getCaps()));
        if (!disableAudio)
            assert(tcpSendBuffer(options["address"], ports::CAPS_OFFSET + (int)options["audioport"], audiofactory::MSG_ID, aTx->getCaps()));

        BLOCK();
        assert(playback::isPlaying() or playback::quitted());

        playback::stop();
    }
    return 0;
}
#include <stdlib.h>

void onExit(void)
{
    LOG_INFO("\x1b[r\x1b[10BBuilt on " << __DATE__ << " at " << __TIME__);
}


int main(int argc, char **argv)
{
    int ret = 0;
    atexit (onExit);
    LOG_INFO("\x1b[0;10r\x1b[2JBuilt on " << __DATE__ << " at " << __TIME__);
    try {
        set_handler();
        ret = pof::run(argc, argv);
    }
    catch (std::exception)
    {
        ret = 1;
    }
    LOG_INFO("\x1b[r\x1b[10BBuilt on " << __DATE__ << " at " << __TIME__);
    return ret;
}

