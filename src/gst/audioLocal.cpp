
// audioLocal.cpp
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
//
// TODO: Client code should just call this with some kind of parameter object which
// specifies number of channels, how to compress it (if at all), and host and port info.

#include <cassert>
#include <iostream>
#include "logWriter.h"

#include "audioLocal.h"
#include "audioSource.h"
#include "pipeline.h"
#include "audioSink.h"

AudioLocal::~AudioLocal()
{
    stop();
    delete sink_;
    delete source_;
}


#if 0
std::string AudioLocal::getCaps() 
{ 
    return sink_->getCaps(); 
}
#endif


void AudioLocal::init_source()
{
    assert(source_ = config_.createSource());
    source_->init();

    init_level();
}


void AudioLocal::init_level()
{
    level_.init();
    GstLinkable::link(*source_, level_);
}


void AudioLocal::init_sink()
{
    assert(sink_ = config_.createSink());
    sink_->init();
    GstLinkable::link(level_, *sink_);   
}

