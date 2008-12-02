
/* videoFactory.h
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

#ifndef _VIDEO_FACTORY_H_
#define _VIDEO_FACTORY_H_

#include "ports.h"
#include "gst/engine.h"
#include <memory>   // for std::auto_ptr

namespace videofactory
{
    static const char *V_SINK = "xvimagesink";
    static const char *V_CODEC = "h264";

    static std::auto_ptr<VideoReceiver> 
    buildVideoReceiver(const char *ip = ports::IP, const char * codec = V_CODEC, const long port = ports::V_PORT, 
            int screen_num = 0, const char *sink = V_SINK);

    static std::auto_ptr<VideoSender> 
    buildVideoSender(const VideoSourceConfig vConfig, 
            const char *ip = ports::IP, const char *codec = V_CODEC, const long port = ports::V_PORT);
}

std::auto_ptr<VideoSender> 
videofactory::buildVideoSender(const VideoSourceConfig vConfig, const char *ip, const char *codec, const long port)
{
    SenderConfig rConfig(codec, ip, port);
    std::auto_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));
    tx->init(); return tx;
}

std::auto_ptr<VideoReceiver> 
videofactory::buildVideoReceiver(const char *ip, const char *codec, const long port, const int screen_num, const char *sink)
{
    if(!sink)
        sink = V_SINK;
    VideoSinkConfig vConfig(sink, screen_num);
    ReceiverConfig rConfig(codec, ip, port, "");
    std::auto_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
    rx->init();
    return rx;
}

#endif // _VIDEO_FACTORY_H_

