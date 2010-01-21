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

#include "util.h"
#include "mapMsg.h"

#include "gst/engine.h"

#include <boost/shared_ptr.hpp>

namespace videofactory
{
    using boost::shared_ptr;
    static const int MSG_ID = 2;

    /// Convert command line options to ipcp
    static void rxOptionsToIPCP(MapMsg &options)
    {
        // FIXME: remove unused keys
        options["port"] = options["videoport"];
        options["codec"] = options["videocodec"];
        options["sink"] = options["videosink"];
        options["device"] = options["videodevice"];
        options["location"] = options["videolocation"];
    }

    /// Convert command line options to ipcp
    static void txOptionsToIPCP(MapMsg &options)
    {
        options["port"] = options["videoport"];
        options["codec"] = options["videocodec"];
        options["source"] = options["videosource"];
        options["device"] = options["videodevice"];
        options["location"] = options["videolocation"];
        options["bitrate"] = options["videobitrate"];
        options["quality"] = options["videoquality"];
    }

    /// Convert command line options to ipcp
    static void localOptionsToIPCP(MapMsg &options)
    {
        options["source"] = options["videosource"];
        options["sink"] = options["videosink"];
        options["device"] = options["videodevice"];
        options["location"] = options["videolocation"];

        // FIXME: unused
        options["bitrate"] = options["videobitrate"];
        options["quality"] = options["videoquality"];
        options["port"] = options["videoport"];
        options["codec"] = options["videocodec"];
    }

    static shared_ptr<VideoReceiver> 
        buildVideoReceiver(MapMsg &msg)
        {
            shared_ptr<VideoSinkConfig> vConfig(new VideoSinkConfig(msg));

            // get caps here, based on codec, capture width and capture height
            std::string caps(CapsParser::getVideoCaps(msg["codec"], msg["width"], msg["height"], msg["aspect-ratio"]));

            shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(msg, caps, MSG_ID)); 

            return shared_ptr<VideoReceiver>(new VideoReceiver(vConfig, rConfig));
        }


    static shared_ptr<VideoSender> 
        buildVideoSender(MapMsg &msg)
        {
            shared_ptr<VideoSourceConfig> vConfig(new VideoSourceConfig(msg));

            shared_ptr<SenderConfig> rConfig(new SenderConfig(msg, MSG_ID)); 
            shared_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));

            rConfig->capsOutOfBand(msg["negotiate-caps"] 
                    or !tx->capsAreCached());

            return tx;
        }

    static shared_ptr<LocalVideo> 
        buildLocalVideo(MapMsg &msg)
        {
            shared_ptr<VideoSourceConfig> sourceConfig(new VideoSourceConfig(msg));
            shared_ptr<VideoSinkConfig> sinkConfig(new VideoSinkConfig(msg));

            return shared_ptr<LocalVideo>(new LocalVideo(sourceConfig, sinkConfig));
        }
}

#endif // _VIDEO_FACTORY_H_

