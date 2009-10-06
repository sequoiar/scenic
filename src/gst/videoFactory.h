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
#include "gutil.h"

#include "gst/engine.h"

#include <boost/shared_ptr.hpp>

namespace videofactory
{
    using boost::shared_ptr;
    static const int MSG_ID = 2;

#ifdef __COMMAND_LINE__
    /// Convert command line options to ipcp
    static MapMsg rxOptionsToIPCP(const OptionArgs &options)
    {
        // FIXME: remove unused keys
        MapMsg ipcpMsg(options.toMapMsg());
        ipcpMsg["port"] = ipcpMsg["videoport"];
        ipcpMsg["codec"] = ipcpMsg["videocodec"];
        ipcpMsg["sink"] = ipcpMsg["videosink"];
        ipcpMsg["device"] = ipcpMsg["videodevice"];
        ipcpMsg["location"] = ipcpMsg["videolocation"];

        return ipcpMsg;
    }

    /// Convert command line options to ipcp
    static MapMsg txOptionsToIPCP(const OptionArgs &options)
    {
        MapMsg ipcpMsg(options.toMapMsg());
        ipcpMsg["port"] = ipcpMsg["videoport"];
        ipcpMsg["codec"] = ipcpMsg["videocodec"];
        ipcpMsg["source"] = ipcpMsg["videosource"];
        ipcpMsg["device"] = ipcpMsg["videodevice"];
        ipcpMsg["location"] = ipcpMsg["videolocation"];
        ipcpMsg["bitrate"] = ipcpMsg["videobitrate"];
        ipcpMsg["quality"] = ipcpMsg["videoquality"];

        return ipcpMsg;
    }
#endif // __COMMAND_LINE__

    static void setRxDefaults(MapMsg &msg)
    {
        if (!msg["multicast-interface"])
            msg["multicast-interface"] = "";

        if (!msg["shared-video-id"])
            msg["shared-video-id"] = "shared_memory";

        if(!msg["screen"])
            msg["screen"] = 0;

        if(!msg["sink"])
            msg["sink"] = "xvimagesink";

        if (!msg["address"])
            msg["address"] = "127.0.0.1";
    }

    static void setTxDefaults(MapMsg &msg)
    {
        if (!msg["device"]) 
            msg["device"] = ""; 

        if (!msg["location"]) 
            msg["location"] = ""; 

        if (!msg["quality"])
            msg["quality"] = 0;

        // Only use quality if we're using theora or no bitrate has been set
        if (msg["quality"])
            if (msg["bitrate"])
            {
                LOG_WARNING("Ignoring quality setting for " << msg["codec"]);
                msg["quality"] = 0;
            }

        // If quality is != 0, then we know that we're using theora from the previous check
        if (!msg["bitrate"])
        {
            if (!msg["quality"])
                msg["bitrate"] = 3000000; // quality and bitrate are mutually exclusive
        }


        if (!msg["camera-number"])
            msg["camera-number"] = -1;

        if (!msg["address"])
            msg["address"] = "127.0.0.1";
    }


    static shared_ptr<VideoReceiver> 
        buildVideoReceiver(MapMsg &msg)
        {
            setRxDefaults(msg);
            shared_ptr<VideoSinkConfig> vConfig(new VideoSinkConfig(msg));

            // get caps here
            std::string caps(CapsParser::getVideoCaps(msg["codec"]));

            shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(msg, caps, MSG_ID)); 

            shared_ptr<VideoReceiver> rx(new VideoReceiver(vConfig, rConfig));
            rx->init();

            return rx;
        }


    static shared_ptr<VideoSender> 
        buildVideoSender(MapMsg &msg)
        {
            setTxDefaults(msg);
            shared_ptr<VideoSourceConfig> vConfig(new VideoSourceConfig(msg));

            shared_ptr<SenderConfig> rConfig(new SenderConfig(msg, MSG_ID)); 
            shared_ptr<VideoSender> tx(new VideoSender(vConfig, rConfig));

            rConfig->capsOutOfBand(msg["caps-out-of-band"] 
                    or !tx->capsAreCached());

            tx->init(); 

            return tx;
        }

}

#endif // _VIDEO_FACTORY_H_

