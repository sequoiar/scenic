/* localAudio.cpp
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

#include "localAudio.h"
#include "pipeline.h"
#include "audioSource.h"
#include "audioConfig.h"
#include "audioLevel.h"
#include "dv1394.h"
#include "gstLinkable.h"

using boost::shared_ptr;

/// Constructor
LocalAudio::LocalAudio(Pipeline &pipeline, 
        shared_ptr<AudioSourceConfig> sourceConfig) : 
    pipeline_(pipeline),
    sourceConfig_(sourceConfig),
    source_(sourceConfig_->createSource(pipeline_)), 
    level_(sourceConfig_->createLevel(pipeline_)),
    fakesink_(pipeline_.makeElement("fakesink", NULL))
{
    if (level_ != 0)
    {
        gstlinkable::link(*source_, *level_);
        gstlinkable::link(*level_, fakesink_);
    }
    else
        gstlinkable::link(*source_, fakesink_);

    /// FIXME: hack for dv1394src
    if (sourceConfig_->sourceString() == "dv1394src")
        Dv1394::Instance(pipeline)->doTimestamp();
}

/// Destructor 
LocalAudio::~LocalAudio()
{
    pipeline_.remove(&fakesink_);
    delete level_;
    delete source_;
}
