/* videoFactory.cpp
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
#include "videoFactory.h"
#include "videoSender.h"
#include "videoReceiver.h"
#include "localVideo.h"

#include "videoConfig.h"
#include "remoteConfig.h"
#include "capsParser.h"

using boost::shared_ptr;
namespace po = boost::program_options;

shared_ptr<VideoReceiver> videofactory::buildVideoReceiver(Pipeline &pipeline, const po::variables_map &options)
{
    shared_ptr<VideoSinkConfig> vConfig(new VideoSinkConfig(options));

    std::string codec(options["videocodec"].as<std::string>());
    std::string remoteHost(options["address"].as<std::string>());
    // FIXME: temporary workaround for #143
    if (remoteHost == "localhost")
        remoteHost = "127.0.0.1";
    int port = options["videoport"].as<int>();
    std::string multicastInterface(options["multicast-interface"].as<std::string>());
    bool negotiateCaps = not options["disable-caps-negotiation"].as<bool>();
    bool enableControls = options["enable-controls"].as<bool>();

    // get caps here, based on codec, capture width and capture height
    std::string caps(CapsParser::getVideoCaps(codec, 
                options["width"].as<int>(), 
                options["height"].as<int>(), 
                options["aspect-ratio"].as<std::string>()));


    shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(codec, 
                remoteHost, port, multicastInterface, negotiateCaps, 
                enableControls, caps)); 

    return shared_ptr<VideoReceiver>(new VideoReceiver(pipeline, vConfig, rConfig));
}


shared_ptr<VideoSender> videofactory::buildVideoSender(Pipeline &pipeline, const po::variables_map &options)
{
    shared_ptr<VideoSourceConfig> vConfig(new VideoSourceConfig(options));
    std::string codec(options["videocodec"].as<std::string>());
    std::string remoteHost(options["address"].as<std::string>());
    // FIXME: temporary workaround for #143
    if (remoteHost == "localhost")
        remoteHost = "127.0.0.1";
    int port = options["videoport"].as<int>();

    shared_ptr<SenderConfig> rConfig(new SenderConfig(pipeline, codec, remoteHost, port)); 
    shared_ptr<VideoSender> tx(new VideoSender(pipeline, vConfig, rConfig));

    rConfig->capsOutOfBand(not options["disable-caps-negotiation"].as<bool>() 
            or !tx->capsAreCached());

    return tx;
}

shared_ptr<LocalVideo> videofactory::buildLocalVideo(Pipeline &pipeline, const po::variables_map &options)
{
    shared_ptr<VideoSourceConfig> sourceConfig(new VideoSourceConfig(options));
    shared_ptr<VideoSinkConfig> sinkConfig(new VideoSinkConfig(options));

    return shared_ptr<LocalVideo>(new LocalVideo(pipeline, sourceConfig, sinkConfig));
}

