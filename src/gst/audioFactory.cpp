/* audioFactory.cpp
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

#include "util.h"

#include "audioFactory.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "capsParser.h"
#include "pipeline.h"

using boost::shared_ptr;
namespace po = boost::program_options;


shared_ptr<AudioSender> audiofactory::buildAudioSender(Pipeline &pipeline, const po::variables_map &options)
{
    shared_ptr<AudioSourceConfig> aConfig(new AudioSourceConfig(options));           

    std::string codec(options["audiocodec"].as<std::string>());
    std::string remoteHost(options["address"].as<std::string>());
    int port = options["audioport"].as<int>();

    shared_ptr<SenderConfig> rConfig(new SenderConfig(pipeline, codec, remoteHost, port));

    shared_ptr<AudioSender> tx(new AudioSender(pipeline, aConfig, rConfig));

    rConfig->capsOutOfBand(not options["disable-caps-negotiation"].as<bool>() or !tx->capsAreCached());

    return tx;
}

shared_ptr<AudioReceiver> audiofactory::buildAudioReceiver(Pipeline &pipeline, const po::variables_map &options)
{
    shared_ptr<AudioSinkConfig> aConfig(new AudioSinkConfig(options));

    std::string codec(options["audiocodec"].as<std::string>());
    std::string remoteHost(options["address"].as<std::string>());
    int port = options["audioport"].as<int>();
    std::string multicastInterface(options["multicast-interface"].as<std::string>());
    bool negotiateCaps = not options["disable-caps-negotiation"].as<bool>();
    bool enableControls = options["enable-controls"].as<bool>();

    std::string caps(CapsParser::getAudioCaps(codec,
                options["numchannels"].as<int>(), pipeline.actualSampleRate()));

    shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(codec, remoteHost, port, 
                multicastInterface, negotiateCaps, enableControls, caps));

    return shared_ptr<AudioReceiver>(new AudioReceiver(pipeline, aConfig, rConfig));
}
