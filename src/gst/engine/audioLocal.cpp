/* audioLocal.cpp
 * Copyright (C) 2009 Société des arts technologiques (SAT)
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

#include "audioLocal.h"
#include "audioSource.h"
#include "pipeline.h"
#include "audioSink.h"


/// Constructor 
AudioLocal::AudioLocal(const AudioSourceConfig srcConfig, const AudioSinkConfig sinkConfig) 
    : srcConfig_(srcConfig), sinkConfig_(sinkConfig), source_(0), level_(), sink_(0) 
{}

/// Destructor 
AudioLocal::~AudioLocal()
{
    delete sink_;
    delete source_;
}


void AudioLocal::init_source()
{
    assert(source_ = srcConfig_.createSource());
    source_->init();

    init_level();
}


void AudioLocal::init_level()
{
    level_.init();
    gstlinkable::link(*source_, level_);
}


void AudioLocal::init_sink()
{
    assert(sink_ = sinkConfig_.createSink());
    sink_->init();
    gstlinkable::link(level_, *sink_);   
}

