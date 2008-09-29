// audioLevel.cpp
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
#include <cmath>

#include "audioLevel.h"
#include "pipeline.h"

AudioLevel::~AudioLevel()
{
    assert(stop());
    pipeline_.remove(&level_);
}

bool AudioLevel::init()
{
    assert(level_ = gst_element_factory_make("level", NULL));
    pipeline_.add(level_);
    // TODO register callback to handle level msg
    return true;
}

void AudioLevel::updateRms(double rmsDb)
{
    rms_ = dbToLinear(rmsDb);
}

double AudioLevel::dbToLinear(double db)
{
    return pow(10, db * 0.05);
}

