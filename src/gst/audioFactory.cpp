/* audioFactory.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
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

#include "audioFactory.h"
#include <boost/program_options.hpp>
#include "audioSender.h"
#include "audioReceiver.h"
#include "localAudio.h"
#include "codec.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "caps/caps_server.h"
#include "util/logWriter.h"
#include "pipeline.h"

using std::tr1::shared_ptr;
namespace po = boost::program_options;

void audiofactory::printMaxChannels(const std::string &codec)
{
    LOG_PRINT(codec << " supports up to " <<
            Encoder::maxChannels(codec) << " channels\n");
}

shared_ptr<AudioSender> audiofactory::buildAudioSender(Pipeline &pipeline, const po::variables_map &options)
{
    shared_ptr<AudioSourceConfig> aConfig(new AudioSourceConfig(options));

    std::string codec(options["audiocodec"].as<std::string>());
    std::string remoteHost(options["address"].as<std::string>());
    // FIXME: temporary workaround for https://bugzilla.gnome.org/show_bug.cgi?id=595840
    if (remoteHost == "localhost")
        remoteHost = "127.0.0.1";
    int port = options["audioport"].as<int>();
    std::string multicastInterface(options["multicast-interface"].as<std::string>());

    shared_ptr<SenderConfig> rConfig(new SenderConfig(pipeline, codec, remoteHost, port, multicastInterface));

    shared_ptr<AudioSender> tx(new AudioSender(pipeline, aConfig, rConfig));

    return tx;
}

shared_ptr<AudioReceiver> audiofactory::buildAudioReceiver(Pipeline &pipeline, const po::variables_map &options)
{
    shared_ptr<AudioSinkConfig> aConfig(new AudioSinkConfig(options));

    std::string codec(options["audiocodec"].as<std::string>());
    std::string remoteHost(options["address"].as<std::string>());
    // FIXME: temporary workaround for ticket #143
    if (remoteHost == "localhost")
        remoteHost = "localhost.localdomain";
    int port = options["audioport"].as<int>();
    std::string multicastInterface(options["multicast-interface"].as<std::string>());

    shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(codec, remoteHost, port,
                multicastInterface));

    return shared_ptr<AudioReceiver>(new AudioReceiver(pipeline, aConfig, rConfig));
}

shared_ptr<LocalAudio> audiofactory::buildLocalAudio(Pipeline &pipeline, const po::variables_map &options)
{
    AudioSourceConfig sourceConfig(options);
    return shared_ptr<LocalAudio>(new LocalAudio(pipeline, sourceConfig));
}

