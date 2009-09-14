/* audioFactory.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _AUDIO_FACTORY_H_
#define _AUDIO_FACTORY_H_

#include "gst/engine.h"
#include <boost/lexical_cast.hpp>

#include "ports.h"


namespace audiofactory
{
    static const int AUDIO_BUFFER_USEC = AudioSinkConfig::DEFAULT_BUFFER_TIME;
    static const int MSG_ID = 1;

    static AudioSender* 
    buildAudioSender_(const AudioSourceConfig aConfig, 
                      const std::string &ip, 
                      const std::string &codec, 
                      int port);

    static AudioReceiver*
    buildAudioReceiver_(const std::string &ip, 
                        const std::string &codec, 
                        int port, 
                        const std::string &sink,
                        const std::string &deviceName,
                        int audioBufferTime,
                        const std::string &multicastInterface,
                        int numChannels);
}


static AudioSender*
audiofactory::buildAudioSender_(const AudioSourceConfig aConfig, 
                                const std::string &ip, 
                                const std::string &codec, 
                                int port)
{
    SenderConfig rConfig(codec, ip, port, MSG_ID);
    AudioSender* tx = new AudioSender(aConfig, rConfig);
    tx->init();
    return tx;
}

static AudioReceiver*
audiofactory::buildAudioReceiver_(const std::string &ip, 
                                  const std::string &codec, 
                                  int port, 
                                  const std::string &sink,
                                  const std::string &deviceName,
                                  int audioBufferTime,
                                  const std::string &multicastInterface,
                                  int numChannels)
{
    using boost::lexical_cast;

    AudioSinkConfig aConfig(sink, deviceName, audioBufferTime);
    std::string profile = codec + "_" + 
        lexical_cast<std::string>(numChannels) + "_" + 
        lexical_cast<std::string>(playback::sampleRate());
    LOG_DEBUG("Looking for profile " << profile);
    std::string caps(CapsParser::getAudioCaps(profile)); // get caps here
    ReceiverConfig rConfig(codec, ip, port, multicastInterface, caps, MSG_ID);

    // FIXME: codec class should have list of codecs that need live caps update
    // FIXME: also we shouldn't have to receive caps for non-stereo 
    if (codec == "vorbis" or caps == "")
    {
        LOG_INFO("Waiting for " << codec << " caps from other host");
        rConfig.receiveCaps();  // wait for new caps from sender
    }

    AudioReceiver* rx = new AudioReceiver(aConfig, rConfig);
    rx->init();
    return rx;
}

#ifdef USE_SMART_PTR
#ifdef HAVE_BOOST
#include <boost/shared_ptr.hpp>   // for boost::shared_ptr
#else
#include <tr1/memory>
#endif
    
namespace audiofactory
{
#define USE_SHARED_PTR
#ifdef HAVE_BOOST
    using namespace boost;
#else
    using namespace std::tr1;
#endif

    static shared_ptr<AudioSender> 
    buildAudioSender(const AudioSourceConfig aConfig, 
                     const std::string &ip, 
                     const std::string &codec, 
                     int port)
    {
        return shared_ptr<AudioSender>(buildAudioSender_(aConfig, ip, codec, port));
    }

    static shared_ptr<AudioReceiver> 
    buildAudioReceiver(const std::string &ip, 
                       const std::string &codec, 
                       int port, 
                       const std::string &sink,
                       const std::string &deviceName,
                       int audioBufferTime,
                       const std::string &multicastInterface,
                       int numChannels)
    {
        return shared_ptr<AudioReceiver>(buildAudioReceiver_(ip, codec, port, sink, 
                    deviceName, audioBufferTime, multicastInterface, numChannels));
    }
}

#endif // USE_SMART_PTR

#endif // _AUDIO_FACTORY_H_

