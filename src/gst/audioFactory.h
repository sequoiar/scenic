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

#include "ports.h"


namespace audiofactory
{
    static const int AUDIO_BUFFER_USEC = AudioSinkConfig::DEFAULT_BUFFER_TIME;
    static const int MSG_ID = 1;

    static AudioSender* 
    buildAudioSender_(const AudioSourceConfig aConfig, 
                      const std::string &ip, 
                      const std::string &codec, 
                      int port,
                      bool capsOutOfBand);

    static AudioReceiver*
    buildAudioReceiver_(const std::string &ip, 
                        const std::string &codec, 
                        int port, 
                        const std::string &sink,
                        const std::string &deviceName,
                        int audioBufferTime,
                        const std::string &multicastInterface,
                        int numChannels,
                        bool capsOutOfBand);
}


static AudioSender*
audiofactory::buildAudioSender_(const AudioSourceConfig aConfig, 
                                const std::string &ip, 
                                const std::string &codec, 
                                int port,
                                bool capsOutOfBand)
{
    SenderConfig rConfig(codec, ip, port, MSG_ID);
    AudioSender* tx = new AudioSender(aConfig, rConfig, capsOutOfBand);
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
                                  int numChannels,
                                  bool capsOutOfBand)
{
    AudioSinkConfig aConfig(sink, deviceName, audioBufferTime);

    std::string caps(CapsParser::getAudioCaps(codec, numChannels, playback::sampleRate())); // get caps here
    ReceiverConfig rConfig(codec, ip, port, multicastInterface, caps, MSG_ID, capsOutOfBand);

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
                     int port, 
                     bool capsOutOfBand)
    {
        return shared_ptr<AudioSender>(buildAudioSender_(aConfig, ip, codec, port, capsOutOfBand));
    }

    static shared_ptr<AudioReceiver> 
    buildAudioReceiver(const std::string &ip, 
                       const std::string &codec, 
                       int port, 
                       const std::string &sink,
                       const std::string &deviceName,
                       int audioBufferTime,
                       const std::string &multicastInterface,
                       int numChannels,
                       bool capsOutOfBand)
    {
        return shared_ptr<AudioReceiver>(buildAudioReceiver_(ip, codec, port, sink, 
                    deviceName, audioBufferTime, multicastInterface, numChannels, capsOutOfBand));
    }
}

#endif // USE_SMART_PTR

#endif // _AUDIO_FACTORY_H_

