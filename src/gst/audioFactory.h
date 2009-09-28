/* audioFactory.h
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

#ifndef _AUDIO_FACTORY_H_
#define _AUDIO_FACTORY_H_

#include "util.h"
#include "mapMsg.h"

#include "gst/engine.h"

#include <boost/shared_ptr.hpp>

namespace audiofactory
{
    using boost::shared_ptr;

    static const int AUDIO_BUFFER_USEC = AudioSinkConfig::DEFAULT_BUFFER_TIME;
    static const int MSG_ID = 1;


#ifdef __COMMAND_LINE__
    /// Convert command line options to ipcp
    static MapMsg rxOptionsToIPCP(const OptionArgs &options)
    {
        // FIXME: remove unused keys
        MapMsg ipcpMsg(options.toMapMsg());
        ipcpMsg["port"] = ipcpMsg["audioport"];
        ipcpMsg["codec"] = ipcpMsg["audiocodec"];
        ipcpMsg["sink"] = ipcpMsg["audiosink"];
        ipcpMsg["device"] = ipcpMsg["audiodevice"];
        ipcpMsg["location"] = ipcpMsg["audiolocation"];

        return ipcpMsg;
    }

    /// Convert command line options to ipcp
    static MapMsg txOptionsToIPCP(const OptionArgs &options)
    {
        MapMsg ipcpMsg(options.toMapMsg());
        ipcpMsg["port"] = ipcpMsg["audioport"];
        ipcpMsg["codec"] = ipcpMsg["audiocodec"];
        ipcpMsg["source"] = ipcpMsg["audiosource"];
        ipcpMsg["device"] = ipcpMsg["audiodevice"];
        ipcpMsg["location"] = ipcpMsg["audiolocation"];
        ipcpMsg["bitrate"] = ipcpMsg["audiobitrate"];

        return ipcpMsg;
    }
#endif // __COMMAND_LINE__

    static void setRxDefaults(MapMsg &msg)
    {
        if (!msg["numchannels"]) 
            msg["numchannels"] = 2;

        if (!msg["device"])
            msg["device"] = "";

        if (!msg["audio-buffer-usec"])
            msg["audio-buffer-usec"] = audiofactory::AUDIO_BUFFER_USEC;

        if (!msg["sink"])
            msg["sink"] = "jackaudiosink";
        
        if (!msg["multicast-interface"])
            msg["multicast-interface"] = "";
    }
    
    static void setTxDefaults(MapMsg &msg)
    {
        if (!msg["numchannels"]) 
            msg["numchannels"] = 2;

        if (!msg["device"])
            msg["device"] = "";

        if (!msg["location"])
            msg["location"] = "";
    }

    static shared_ptr<AudioSender> 
        buildAudioSender(MapMsg &msg)
        {
            setTxDefaults(msg);

            shared_ptr<AudioSourceConfig> aConfig(new AudioSourceConfig(msg));           

            shared_ptr<SenderConfig> rConfig(new SenderConfig(msg["codec"], 
                        msg["address"], msg["port"], MSG_ID));

            shared_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));

            rConfig->capsOutOfBand(msg["caps-out-of-band"] 
                    or !tx->capsAreCached());

            tx->init();
            return tx;
        }

    static shared_ptr<AudioReceiver> 
        buildAudioReceiver(MapMsg &msg)
        {
            setRxDefaults(msg);

            int bufferTime = msg["audio-buffer-usec"];  // FIXME: hack to get around type problem
            shared_ptr<AudioSinkConfig> aConfig(new AudioSinkConfig(msg["sink"],
                        msg["device"], bufferTime));

            std::string caps(CapsParser::getAudioCaps(msg["codec"],
                        msg["numchannels"], playback::sampleRate()));

            shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(msg["codec"],
                        msg["address"], msg["port"],
                        msg["multicast-interface"],
                        caps, MSG_ID, msg["caps-out-of-band"]));

            shared_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
            rx->init();
            return rx;
        }
}


#endif // _AUDIO_FACTORY_H_

