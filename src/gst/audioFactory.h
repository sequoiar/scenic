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
#include "engine/pipeline.h"

#include <boost/shared_ptr.hpp>

namespace audiofactory
{
    using boost::shared_ptr;

    static const int MSG_ID = 1;

    /// Convert command line options to ipcp
    static void rxOptionsToIPCP(MapMsg &options)
    {
        options["port"] = options["audioport"];
        options["codec"] = options["audiocodec"];
        options["sink"] = options["audiosink"];
        options["device"] = options["audiodevice"];
        options["location"] = options["audiolocation"];
    }

    /// Convert command line options to ipcp
    static void txOptionsToIPCP(MapMsg &options)
    {
        options["port"] = options["audioport"];
        options["codec"] = options["audiocodec"];
        options["source"] = options["audiosource"];
        options["device"] = options["audiodevice"];
        options["location"] = options["audiolocation"];
    }

    static shared_ptr<AudioSender> 
        buildAudioSender(Pipeline &pipeline, MapMsg &msg)
        {
            shared_ptr<AudioSourceConfig> aConfig(new AudioSourceConfig(msg));           

            shared_ptr<SenderConfig> rConfig(new SenderConfig(pipeline, msg, MSG_ID));

            shared_ptr<AudioSender> tx(new AudioSender(pipeline, aConfig, rConfig));

            rConfig->capsOutOfBand(msg["negotiate-caps"] 
                    or !tx->capsAreCached());

            return tx;
        }

    static shared_ptr<AudioReceiver> 
        buildAudioReceiver(Pipeline &pipeline, MapMsg &msg)
        {
            shared_ptr<AudioSinkConfig> aConfig(new AudioSinkConfig(msg));

            std::string caps(CapsParser::getAudioCaps(msg["codec"],
                        msg["numchannels"], pipeline.actualSampleRate()));

            shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(msg, caps, MSG_ID));

            return shared_ptr<AudioReceiver>(new AudioReceiver(pipeline, aConfig, rConfig));
        }
}

#endif // _AUDIO_FACTORY_H_
