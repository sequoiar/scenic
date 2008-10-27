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
#include <cstring>
#include <gst/gst.h>

#include "audioLevel.h"
#include "pipeline.h"
#include "logWriter.h"
#include "mapMsg.h"

#include <sstream>
#include <iostream>
#include <iterator>

AudioLevel::~AudioLevel()
{
    stop();
    pipeline_.remove(&level_);
}

void AudioLevel::init()
{
    assert(level_ = gst_element_factory_make("level", NULL));
    pipeline_.add(level_);
    g_object_set(G_OBJECT(level_), "interval", interval_, "message", emitMessages_, NULL);

    // register this level to handle level msg
    pipeline_.subscribe(this);
}


void AudioLevel::emitMessages(bool doEmit)
{
    emitMessages_ = doEmit;
    g_object_set(G_OBJECT(level_), "message", emitMessages_, NULL);
}


void AudioLevel::updateRms(double rmsDb, size_t channelIdx)
{
    if (channelIdx == rmsValues_.size())
        rmsValues_.push_back(dbToLinear(rmsDb));    // new channel
    else if (channelIdx > rmsValues_.size())
        LOG_WARNING("Invalid channel index, discarding rms value");
    else
        rmsValues_[channelIdx] = dbToLinear(rmsDb);
}

double AudioLevel::dbToLinear(double db)
{
    return pow(10, db * 0.05);
}

bool AudioLevel::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);

    if (strncmp(name, "level", strlen("level")) == 0) {   // this is level's msg
        guint channels;
        double rmsDb;
        const GValue *list;
        const GValue *value;

        // we can get the number of channels as the length of the value list
        list = gst_structure_get_value (s, "rms");
        channels = gst_value_list_get_size (list);

        for (size_t channelIdx = 0; channelIdx < channels; ++channelIdx) {
            list = gst_structure_get_value(s, "rms");
            value = gst_value_list_get_value(list, channelIdx);
            rmsDb = g_value_get_double(value);
            updateRms(rmsDb, channelIdx);
        }
        // TODO: post to static function with mapmsg
        //print();
        post();

        return true;
    }

    return false;           // this wasn't our msg, someone else should handle it
}


void AudioLevel::print() const
{
    std::ostringstream os;
    std::copy(rmsValues_.begin(), rmsValues_.end(), std::ostream_iterator<double>(os, " "));

    LOG_DEBUG("rms values: " << os.str());
}

void AudioLevel::post() const
{
    MapMsg mapMsg("levels");
    //int channelIdx = 1;

    mapMsg["values"] = rmsValues_;
    MSG::post(mapMsg);
#if 0
    for (std::vector<double>::const_iterator iter = rmsValues_.begin(); iter != rmsValues_.end(); ++iter)
    {
        std::stringstream key;
        key << "channel" << channelIdx;
        mapMsg[key.str()] = *iter;
        ++channelIdx;
        MSG::post(mapMsg);
    }
#endif
}


void AudioLevel::interval(unsigned long long newInterval)
{
    interval_ = newInterval;
    g_object_set (G_OBJECT(level_), "interval", interval_, "message", emitMessages_, NULL);
}

