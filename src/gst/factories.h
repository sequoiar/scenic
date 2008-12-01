
/* factories.h
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

#ifndef _FACTORIES_H_
#define _FACTORIES_H_

#include "gst/engine.h"
#include <memory>   // for std::auto_ptr
#include "tcp/singleBuffer.h"


namespace ports {
        const long A_PORT = 10000;
        const long V_PORT = 11000;
        const long CAPS_PORT = 12000;
}

#define DEFAULT_SINK "xvimagesink"

namespace factories 
{
    std::auto_ptr<AudioSender> 
    buildAudioSender(const AudioSourceConfig aConfig, const char* ip, const char *codec, const long port = ports::A_PORT);

    std::auto_ptr<AudioReceiver> 
    buildAudioReceiver(const char *ip, const char * codec, const long port = ports::A_PORT, 
                       const char *sink = "jackaudiosink");

    std::auto_ptr<VideoReceiver> 
    buildVideoReceiver(const char *ip, const char * codec, const long port = ports::V_PORT, 
                       int screen_num = 0, const char *sink = DEFAULT_SINK);
    
    std::auto_ptr<VideoSender> buildVideoSender(const VideoSourceConfig vConfig, 
            const char *ip, const char *codec, const long port = ports::V_PORT);
}

std::auto_ptr<AudioSender> 
factories::buildAudioSender(const AudioSourceConfig aConfig, const char* ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::auto_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<AudioReceiver> 
factories::buildAudioReceiver(const char *ip, const char *codec, const long port, const char *sink)
{
    if(!sink)
        sink = DEFAULT_SINK;
    AudioSinkConfig aConfig(sink);
    ReceiverConfig rConfig(codec, ip, port, tcpGetBuffer(ports::CAPS_PORT));
    std::auto_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
    rx->init();
    return rx;
}

std::auto_ptr<VideoSender> factories::buildVideoSender(const VideoSourceConfig vConfig, const char *ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
    tx->init();
    return tx;
}

std::auto_ptr<VideoReceiver> 
factories::buildVideoReceiver(const char *ip, const char *codec, const long port, const int screen_num, const char *sink)
{
    VideoSinkConfig vConfig(sink, screen_num);
    ReceiverConfig rConfig(codec, ip, port, "");
    std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
    rx->init();
    return rx;
}

#endif // _FACTORIES_H_

