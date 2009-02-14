/* audioFactory.h
 * Copyright 2008 Koya Charles & Tristan Matthews 
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
#include "tcp/singleBuffer.h"

#include "ports.h"


namespace audiofactory
{
    static const std::string &A_SINK = "jackaudiosink";
    static const std::string &A_CODEC = "raw";
    static const int MSG_ID = 1;

    static AudioSender* 
    buildAudioSender_(const AudioSourceConfig aConfig, 
                      const std::string &ip = ports::IP, 
                      const std::string &codec = A_CODEC, 
                      int port = ports::A_PORT);

    static AudioReceiver*
    buildAudioReceiver_(const std::string &ip = ports::IP, 
                        const std::string &codec = A_CODEC, 
                        int port = ports::A_PORT, 
                        const std::string &sink = A_SINK);
}


static AudioSender*
audiofactory::buildAudioSender_(const AudioSourceConfig aConfig, 
                                const std::string &ip, 
                                const std::string &codec, 
                                int port)
{
    SenderConfig rConfig(codec, ip, port);
    AudioSender* tx = new AudioSender(aConfig, rConfig);
    tx->init();
    return tx;
}

static AudioReceiver*
audiofactory::buildAudioReceiver_(const std::string &ip, 
                                  const std::string &codec, 
                                  int port, 
                                  const std::string &sink)
{
    AudioSinkConfig aConfig(sink);
    int id;
    ReceiverConfig rConfig(codec, ip, port, tcpGetBuffer(ports::AUDIO_CAPS_PORT, id)); // get caps from remote sender
    assert(id == MSG_ID);
    AudioReceiver* rx = new AudioReceiver(aConfig, rConfig);
    rx->init();
    return rx;
}

#ifdef USE_SMART_PTR
#ifdef CONFIG_BOOST
#include <boost/shared_ptr.hpp>   // for boost::shared_ptr
#else
#include <tr1/memory>
#endif
    
namespace audiofactory
{
#define USE_SHARED_PTR
#ifdef CONFIG_BOOST
    using namespace boost;
#else
    using namespace std::tr1;
#endif

    static shared_ptr<AudioSender> 
    buildAudioSender(const AudioSourceConfig aConfig, 
                     const std::string &ip = ports::IP, 
                     const std::string &codec = A_CODEC, 
                     unsigned long port = ports::A_PORT)
    {
        assert(port != ports::VIDEO_CAPS_PORT && port != ports::AUDIO_CAPS_PORT); 
        return shared_ptr<AudioSender>(buildAudioSender_(aConfig, ip, codec, port));
    }

    static shared_ptr<AudioReceiver> 
    buildAudioReceiver(const std::string &ip = ports::IP, 
                       const std::string &codec = A_CODEC, 
                       unsigned long port = ports::A_PORT, 
                       const std::string &sink = A_SINK)
    {
        assert(port != ports::VIDEO_CAPS_PORT && port != ports::AUDIO_CAPS_PORT); 
        return shared_ptr<AudioReceiver>(buildAudioReceiver_(ip,codec,port,sink));
    }
}

#endif // USE_SMART_PTR

#endif // _AUDIO_FACTORY_H_

