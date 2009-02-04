/* audioFactory.h
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

#ifndef _AUDIO_FACTORY_H_
#define _AUDIO_FACTORY_H_

#include "gst/engine.h"
#include <boost/shared_ptr.hpp>   // for boost::shared_ptr
#include "tcp/singleBuffer.h"

#include "ports.h"

#include "gst/audioFactoryInternal.h"

namespace audiofactory
{

    static boost::shared_ptr<AudioSender> 
    buildAudioSender(const AudioSourceConfig aConfig, 
                     const std::string &ip = ports::IP, 
                     const std::string &codec = A_CODEC, 
                     int port = ports::A_PORT)
    {
        return boost::shared_ptr<AudioSender>(buildAudioSender_(aConfig, ip, codec, port));
    }

    static boost::shared_ptr<AudioReceiver> 
    buildAudioReceiver(const std::string &ip = ports::IP, 
                       const std::string &codec = A_CODEC, 
                       int port = ports::A_PORT, 
                       const std::string &sink = A_SINK)
    {
        return boost::shared_ptr<AudioReceiver>(buildAudioReceiver_(ip,codec,port,sink));
    }

}


#endif // _AUDIO_FACTORY_H_

