
// videoLocal.cpp
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

#include <cassert>

#include "pipeline.h"
#include "gstLinkable.h"
#include "videoLocal.h"
#include "videoSource.h"
#include "videoSink.h"
#include "videoConfig.h"
#include "logWriter.h"


VideoLocal::~VideoLocal()
{
    assert(stop());
    delete sink_;
    delete source_;
}


void VideoLocal::init_source()
{
    assert(source_ = config_.createSource());
    source_->init();
}


void VideoLocal::init_sink()
{
        assert(sink_ = config_.createSink());
        sink_->init();
        if (config_.fileExists())       // bad design
            sink_->makeSyncTrue();
        else
            GstLinkable::link(*source_, *sink_);   // FIXME: this shouldn't happen for VideoFileSource
}


bool VideoLocal::start()
{
    GstBase::start();
    pipeline_.wait_until_playing(); // otherwise it doesn't know it's playing
    sink_->showWindow();
    return true;
}

