/*
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

#include "video_factory.h"
#include <boost/program_options.hpp>
#include "video_sender.h"
#include "video_receiver.h"
#include "local_video.h"

#include "video_config.h"
#include "remote_config.h"
#include "caps/caps_server.h"

using std::tr1::shared_ptr;
namespace po = boost::program_options;

shared_ptr<VideoReceiver> videofactory::buildVideoReceiver(Pipeline &pipeline, const po::variables_map &options)
{
    using std::string;
    shared_ptr<VideoSinkConfig> vConfig(new VideoSinkConfig(options));

    shared_ptr<ReceiverConfig> rConfig(new ReceiverConfig(options["videocodec"].as<string>(),
                options["address"].as<string>(), options["videoport"].as<int>(),
                options["multicast-interface"].as<string>(),
                options["jitterbuffer"].as<int>()));

    return shared_ptr<VideoReceiver>(new VideoReceiver(pipeline, vConfig, rConfig));
}


shared_ptr<VideoSender> videofactory::buildVideoSender(Pipeline &pipeline, const po::variables_map &options)
{
    using std::string;
    shared_ptr<VideoSourceConfig> vConfig(new VideoSourceConfig(options));

    shared_ptr<SenderConfig> sConfig(new SenderConfig(pipeline, options["videocodec"].as<string>(),
                options["address"].as<string>(), options["videoport"].as<int>(),
                options["multicast-interface"].as<string>()));

    shared_ptr<VideoSender> tx(new VideoSender(pipeline, vConfig, sConfig));

    return tx;
}

shared_ptr<LocalVideo> videofactory::buildLocalVideo(Pipeline &pipeline, const po::variables_map &options)
{
    VideoSourceConfig sourceConfig(options);
    VideoSinkConfig sinkConfig(options);

    return shared_ptr<LocalVideo>(new LocalVideo(pipeline, sourceConfig, sinkConfig));
}

