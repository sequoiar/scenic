/* localVideo.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include "localVideo.h"
#include <boost/shared_ptr.hpp>
#include <pipeline.h>

#include "videoSource.h"
#include "videoScale.h"
#include "videoSink.h"

using boost::shared_ptr;

/// Constructor
LocalVideo::LocalVideo(Pipeline &pipeline, 
        shared_ptr<VideoSourceConfig> sourceConfig, 
        shared_ptr<VideoSinkConfig> sinkConfig) : 
    pipeline_(pipeline),
    sourceConfig_(sourceConfig),
    sinkConfig_(sinkConfig),
    source_(sourceConfig_->createSource(pipeline_)), 
    colourspace_(pipeline_.makeElement("ffmpegcolorspace", NULL)),
    videoscale_(sinkConfig_->createVideoScale(pipeline_)),
    sink_(sinkConfig_->createSink(pipeline_))
{
    if (sourceConfig_->sourceString() != "dc1394src")
    {
        gstlinkable::link(*source_, *videoscale_);
        gstlinkable::link(*videoscale_, *sink_);
    }
    else
    {
        gstlinkable::link(*source_, colourspace_);
        gstlinkable::link(colourspace_, *videoscale_);
        gstlinkable::link(*videoscale_, *sink_);
    }
}

/// Destructor 
LocalVideo::~LocalVideo()
{
    delete sink_;
    pipeline_.remove(&colourspace_);
    delete source_;
}

