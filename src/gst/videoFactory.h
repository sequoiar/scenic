/* videoFactory.h
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

#ifndef _VIDEO_FACTORY_H_
#define _VIDEO_FACTORY_H_

#include "ports.h"
#include "gst/engine.h"

namespace videofactory
{
    static const int MSG_ID = 2;

    static VideoReceiver* 
    buildVideoReceiver_(const std::string &ip, 
            const std::string &codec, 
            int port, 
            int screen_num, 
            const std::string &sink, 
            bool deinterlace, 
            const std::string &sharedVideoId, 
            const std::string &multicastInterface,
            bool capsOutOfBand);

    static VideoSender* 
    buildVideoSender_(const VideoSourceConfig vConfig, 
            const std::string &ip, const std::string &codec, int port, bool capsOutOfBand);
}



VideoSender* 
videofactory::buildVideoSender_(const VideoSourceConfig vConfig, 
                                const std::string &ip, 
                                const std::string &codec, 
                                int port,
                                bool capsOutOfBand)
{
    SenderConfig rConfig(codec, ip, port, MSG_ID); 
    VideoSender* tx = new VideoSender(vConfig, rConfig, capsOutOfBand);
    tx->init(); 
    return tx;
}


VideoReceiver*
videofactory::buildVideoReceiver_(const std::string &ip, 
                                  const std::string &codec, 
                                  int port, 
                                  int screen_num, 
                                  const std::string &sink,
                                  bool deinterlace,
                                  const std::string &sharedVideoId,
                                  const std::string &multicastInterface,
                                  bool capsOutOfBand)
{
    assert(!sink.empty());
    VideoSinkConfig vConfig(sink, screen_num, deinterlace, sharedVideoId);
    std::string caps(CapsParser::getVideoCaps(codec)); // get caps here
    
    ReceiverConfig rConfig(codec, ip, port, multicastInterface, caps, MSG_ID); 
    
    if (caps == "" or capsOutOfBand) // couldn't find caps, need them from other host
    {
        LOG_INFO("Waiting for " << codec << " caps from other host");
        rConfig.receiveCaps();  // wait for new caps from sender
    }

    VideoReceiver* rx = new VideoReceiver(vConfig, rConfig);
    rx->init();
    return rx;
}

#ifdef USE_SMART_PTR
#ifdef HAVE_BOOST
#include <boost/shared_ptr.hpp>   // for boost::shared_ptr
#else
#include <tr1/memory>
#endif

namespace videofactory
{
#ifdef HAVE_BOOST
    using namespace boost;
#else
    using namespace std::tr1;
#endif

    static shared_ptr<VideoReceiver> 
    buildVideoReceiver(const std::string &ip, 
                       const std::string &codec, 
                       int port, 
                       int screen_num, 
                       const std::string &sink,
                       bool deinterlace,
                       const std::string &sharedVideoId,
                       const std::string &multicastInterface,
                       bool capsOutOfBand)
    {
        return shared_ptr<VideoReceiver>(buildVideoReceiver_(ip, codec, port, 
                    screen_num, sink, deinterlace, sharedVideoId, multicastInterface, capsOutOfBand));
    }

    static shared_ptr<VideoSender> 
    buildVideoSender(const VideoSourceConfig vConfig, 
                     const std::string &ip, 
                     const std::string &codec, 
                     int port,
                     bool capsOutOfBand)
    {
        return shared_ptr<VideoSender>(buildVideoSender_(vConfig, ip, codec, port, capsOutOfBand));
    }

}

#endif //USE_SHARED_PTR
#endif // _VIDEO_FACTORY_H_

