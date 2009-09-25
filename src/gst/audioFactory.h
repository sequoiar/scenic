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

#include "gst/engine.h"

#include <boost/shared_ptr.hpp>

using boost::shared_ptr;

namespace audiofactory
{
    static const int AUDIO_BUFFER_USEC = AudioSinkConfig::DEFAULT_BUFFER_TIME;
    static const int MSG_ID = 1;

    static shared_ptr<AudioSender> 
        buildAudioSender(shared_ptr<AudioSourceConfig> aConfig, 
                const std::string &ip, 
                const std::string &codec, 
                int port, 
                bool capsOutOfBand)
        {
            shared_ptr<SenderConfig> rConfig(new SenderConfig(codec, ip, port, MSG_ID));
            shared_ptr<AudioSender> tx(new AudioSender(aConfig, rConfig));
            rConfig->capsOutOfBand(capsOutOfBand or !tx->capsAreCached());
            tx->init();
            return tx;
        }

    static shared_ptr<AudioReceiver> 
        buildAudioReceiver(const std::string &ip, 
                const std::string &codec, 
                int port, 
                const std::string &sink,
                const std::string &deviceName,
                int audioBufferTime,
                const std::string &multicastInterface,
                int numChannels,
                bool capsOutOfBand)
        {
            shared_ptr<AudioSinkConfig> aConfig(new AudioSinkConfig(sink, deviceName, audioBufferTime));

            std::string caps(CapsParser::getAudioCaps(codec, numChannels, playback::sampleRate())); // get caps here
            shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(codec, ip, port, 
                        multicastInterface, caps, MSG_ID, capsOutOfBand));

            shared_ptr<AudioReceiver> rx(new AudioReceiver(aConfig, rConfig));
            rx->init();
            return rx;
        }
}


#endif // _AUDIO_FACTORY_H_

