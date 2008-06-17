
// videoSender.cpp
// Copyright 2008 Koya Charles & Tristan Matthews 
//     
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//


#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <gst/gst.h>

#include "videoSender.h"

VideoSender::VideoSender(const VideoConfig& config) : config_(config)
{
    // empty
}



VideoSender::~VideoSender() 
{
    // empty
}



bool VideoSender::init()
{
    GError* error = NULL;
    std::string launchStr = config_.source();

    if (!launchStr.compare("dv1394src")) // need to demux and decode dv first
        launchStr += " ! dvdemux name=demux demux. ! queue ! dvdec";

    if (!config_.codec().compare("h264"))
        launchStr += " ! ffmpegcolorspace ! x264enc bitrate=2048 byte-stream=true threads=4";
    
    if (config_.isNetworked())
    {
        launchStr += " ! rtph264pay ! udpsink host=" + config_.remoteHost(); 

        std::stringstream istream;
        istream << " port = " << config_.port();           
        launchStr += istream.str();     // get port number into launch string
    }
    else // local test only
        launchStr += " ! xvimagesink sync=false"; 

    pipeline_ = gst_parse_launch(launchStr.c_str(), &error);
    assert(pipeline_);

    make_verbose();

    // FIXME: this method should really check the pipeline
    return check_pipeline();
}



bool VideoSender::start()
{
    if (config_.port())
    {
        std::cout << "Sending media on port " << config_.port() << " to host " << config_.remoteHost()
            << std::endl;
    }

    return MediaBase::start();
}

