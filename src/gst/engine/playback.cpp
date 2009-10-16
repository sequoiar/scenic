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

#include "util.h"

#include "playback.h"
#include "pipeline.h"
#include "gutil.h"
#include "mapMsg.h"


void playback::start()
{
    Pipeline::Instance()->start();
}


void playback::stop()
{
    Pipeline::Instance()->stop();
}

void playback::pause()
{
    Pipeline::Instance()->pause();
}

bool playback::isPlaying()
{ 
    return Pipeline::Instance()->isPlaying(); 
}

void playback::quit()
{
    stop();
    Pipeline::Instance()->notifyQuitted();
    gutil::killMainLoop();
    MapMsg quitMsg("quit");
    quitMsg.post();
}

void playback::makeVerbose()
{
    Pipeline::Instance()->makeVerbose();
}

bool playback::quitted()
{
    return Pipeline::Instance()->quitted();
}

int playback::sampleRate()
{
    return Pipeline::SAMPLE_RATE;
}

