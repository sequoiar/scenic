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
#include "pipeline.h"

#include "videoSource.h"
#include "videoScale.h"
#include "videoFlip.h"
#include "videoSink.h"

#include "dv1394.h"

using boost::shared_ptr;

/// Constructor
LocalVideo::LocalVideo(Pipeline &pipeline, 
        shared_ptr<VideoSourceConfig> sourceConfig, 
        shared_ptr<VideoSinkConfig> sinkConfig) : 
    pipeline_(pipeline),
    sourceConfig_(sourceConfig),
    sinkConfig_(sinkConfig),
    source_(sourceConfig_->createSource(pipeline_)), 
    colourspace_(0),
    videoscale_(sinkConfig_->createVideoScale(pipeline_)),
    videoflip_(sinkConfig_->flipMethod() != "none" ? sinkConfig_->createVideoFlip(pipeline_) : 0),
    sink_(sinkConfig_->createSink(pipeline_))
{
    // dc1394src needs an extra colourspace if not being encoded or flipped
    if (sourceConfig_->sourceString() == "dc1394src" and videoflip_ == 0)
    {
        colourspace_ = pipeline_.makeElement("ffmpegcolorspace", NULL);
        gstlinkable::link(*source_, colourspace_);
        gstlinkable::link(colourspace_, *videoscale_);
    }
    else
    {
        gstlinkable::link(*source_, *videoscale_);
    }

    if (videoflip_ != 0)
    {
        gstlinkable::link(*videoscale_, *videoflip_);
        gstlinkable::link(*videoflip_, *sink_);
    }
    else
        gstlinkable::link(*videoscale_, *sink_);

    /// FIXME: hack for dv1394src
    if (sourceConfig_->sourceString() == "dv1394src")
        Dv1394::Instance(pipeline)->doTimestamp();
}

/// Destructor 
LocalVideo::~LocalVideo()
{
    delete sink_;
    pipeline_.remove(&colourspace_);
    delete videoflip_;
    delete videoscale_;
    delete source_;
}

