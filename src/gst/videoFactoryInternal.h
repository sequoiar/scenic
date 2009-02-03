
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

#ifndef _VIDEO_FACTORY_INTERNAL_H_
#define _VIDEO_FACTORY_INTERNAL_H_

#include "ports.h"
#include "gst/engine.h"
#include <boost/shared_ptr.hpp>   // for boost::shared_ptr
#include "tcp/singleBuffer.h"

namespace videofactory
{
    static const char *V_SINK = "xvimagesink";
    static const char *V_CODEC = "mpeg4";
    static const int MSG_ID = 2;

    static VideoReceiver* 
    buildVideoReceiver_(const char *ip = ports::IP, const char * codec = V_CODEC, int port = ports::V_PORT, 
            int screen_num = 0, const char *sink = V_SINK);

    static VideoSender* 
    buildVideoSender_(const VideoSourceConfig vConfig, 
            const char *ip = ports::IP, const char *codec = V_CODEC, int port = ports::V_PORT);


}

VideoSender* 
videofactory::buildVideoSender_(const VideoSourceConfig vConfig, const char *ip, const char *codec, int port)
{
    SenderConfig rConfig(codec, ip, port);
    VideoSender* tx = new VideoSender(vConfig, rConfig);
    tx->init(); 
    return tx;
}

VideoReceiver*
videofactory::buildVideoReceiver_(const char *ip, const char *codec, int port, int screen_num, const char *sink)
{
    if(!sink)
        sink = V_SINK;
    VideoSinkConfig vConfig(sink, screen_num);
    int id;
    ReceiverConfig rConfig(codec, ip, port, tcpGetBuffer(ports::VIDEO_CAPS_PORT, id)); // get caps from remote sender
    assert(id == MSG_ID);
    VideoReceiver* rx = new VideoReceiver(vConfig, rConfig);
    rx->init();
    return rx;
}

#endif // _VIDEO_FACTORY_H_

