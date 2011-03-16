/* mediaBase.cpp
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

#include "playback.h"
#include "pipeline.h"
#include "gtk_utils.h"


Playback::Playback(const Pipeline &pipeline) : pipeline_(pipeline)
{
}

void Playback::start()
{
    pipeline_.start();
}


void Playback::stop()
{
    pipeline_.stop();
}

void Playback::pause()
{
    pipeline_.pause();
}

bool Playback::isPlaying()
{ 
    return pipeline_.isPlaying(); 
}

void Playback::quit()
{
    pipeline_.quit();
}

void Playback::makeVerbose()
{
    pipeline_.makeVerbose();
}

int Playback::sampleRate()
{
    return pipeline_.actualSampleRate();
}

