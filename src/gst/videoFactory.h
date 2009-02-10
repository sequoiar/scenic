
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
#include <boost/shared_ptr.hpp>   // for boost::shared_ptr
#include "tcp/singleBuffer.h"

#include "videoFactoryInternal.h"

namespace videofactory
{

    static boost::shared_ptr<VideoReceiver> 
    buildVideoReceiver(const std::string &ip = ports::IP, 
                       const std::string &codec = V_CODEC, 
                       int port = ports::V_PORT, 
                       int screen_num = 0, 
                       const std::string &sink = V_SINK)
    {
        assert(port != VIDEO_CAPS_PORT && port != AUDIO_CAPS_PORT); 
        return boost::shared_ptr<VideoReceiver>(buildVideoReceiver_(ip, codec, port, screen_num, sink));
    }

    static boost::shared_ptr<VideoSender> 
    buildVideoSender(const VideoSourceConfig vConfig, 
                     const std::string &ip = ports::IP, 
                     const std::string &codec = V_CODEC, 
                     int port = ports::V_PORT)
    {
        assert(port != VIDEO_CAPS_PORT && port != AUDIO_CAPS_PORT); 
        return boost::shared_ptr<VideoSender>(buildVideoSender_(vConfig,ip,codec,port));
    }

}


#endif // _VIDEO_FACTORY_H_

