
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
#include <tr1/memory>   // for std::tr1::shared_ptr
#include "tcp/singleBuffer.h"

#include "ports.h"

namespace audiofactory
{
    static const char* A_SINK = "jackaudiosink";
    static const char* A_CODEC = "raw";
    static const int MSG_ID = 1;

    static std::tr1::shared_ptr<AudioSender> 
    buildAudioSender(const AudioSourceConfig aConfig, const char* ip = ports::IP, const char *codec = A_CODEC, 
            const long port = ports::A_PORT);

    static std::tr1::shared_ptr<AudioReceiver> 
    buildAudioReceiver(const char *ip = ports::IP, const char * codec = A_CODEC, const long port = ports::A_PORT, 
                       const char *sink = A_SINK);
}

std::tr1::shared_ptr<AudioSender> 
audiofactory::buildAudioSender(const AudioSourceConfig aConfig, const char* ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::tr1::shared_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    tx->init();
    return tx;
}

std::tr1::shared_ptr<AudioReceiver> 
audiofactory::buildAudioReceiver(const char *ip, const char *codec, const long port, const char *sink)
{
    AudioSinkConfig aConfig(sink);
    int id;
    ReceiverConfig rConfig(codec, ip, port, tcpGetBuffer(ports::AUDIO_CAPS_PORT, id)); // get caps from remote sender
    assert(id == MSG_ID);
    std::tr1::shared_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}


#endif // _AUDIO_FACTORY_H_

