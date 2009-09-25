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

#include "gst/engine.h"

#include <boost/shared_ptr.hpp>


namespace videofactory
{
    using boost::shared_ptr;
    static const int MSG_ID = 2;

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
            shared_ptr<VideoSinkConfig> vConfig(new VideoSinkConfig(sink, screen_num, 
                        deinterlace, sharedVideoId));
            std::string caps(CapsParser::getVideoCaps(codec)); // get caps here

            shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(codec, ip, port, 
                        multicastInterface, caps, MSG_ID, capsOutOfBand)); 

            shared_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
            rx->init();

            return rx;
        }


    static shared_ptr<VideoSender> 
        buildVideoSender(shared_ptr<VideoSourceConfig> vConfig, 
                const std::string &ip, 
                const std::string &codec, 
                int port,
                bool capsOutOfBand)
        {
            shared_ptr<SenderConfig> rConfig(new SenderConfig(codec, ip, port, MSG_ID)); 
            shared_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));

            rConfig->capsOutOfBand(capsOutOfBand or !tx->capsAreCached());

            tx->init(); 

            return tx;
        }

}

#endif // _VIDEO_FACTORY_H_

