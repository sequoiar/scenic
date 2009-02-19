/* audioLevel.cpp
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

#include <cstring>
#include <cmath>
#include <gst/gst.h>
#include "audioLevel.h"
#include "pipeline.h"
#include "mapMsg.h"

#include <iterator>


/** Constructor sets by default emitMessages to true 
 * and message interval to one second */
AudioLevel::AudioLevel() : 
    level_(0), emitMessages_(true) {}

/// Destructor 
AudioLevel::~AudioLevel()
{
    Pipeline::Instance()->remove(&level_);
}

/// Class initializer 
void AudioLevel::init()
{
    level_ = Pipeline::Instance()->makeElement("level", NULL);
    g_object_set(G_OBJECT(level_), "interval", 1000000000LL, "message", emitMessages_, NULL);

    // register this level to handle level msg
    Pipeline::Instance()->subscribe(this);
}


/**
 * Toggles whether or not this AudioLevel will post messages on the bus. */
void AudioLevel::emitMessages(bool doEmit)
{
    emitMessages_ = doEmit;
    g_object_set(G_OBJECT(level_), "message", emitMessages_, NULL);
}


/// Converts from decibel to linear (0.0 to 1.0) scale. 
double AudioLevel::dbToLinear(double db)
{
    return pow(10, db * 0.05);
}

/** 
 * The level message is posted on the bus by the level element, 
 * received by this AudioLevel, and dispatched. */
bool AudioLevel::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);
    std::vector<double> rmsValues;

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
            rmsValues.push_back(dbToLinear(rmsDb));    // new channel
        }
        post(rmsValues);

        return true;
    }

    return false;           // this wasn't our msg, someone else should handle it
}


/// Prints current rms values through the LogWriter system. 
void AudioLevel::print(const std::vector<double> &rmsValues) const
{
    std::ostringstream os;
    std::copy(rmsValues.begin(), rmsValues.end(), std::ostream_iterator<double>(os, " "));

    LOG_DEBUG("rms values: " << os.str());
}


/// Posts the rms values to be handled at a higher level by the MapMsg system. 
void AudioLevel::post(const std::vector<double> &rmsValues) const
{
    MapMsg mapMsg("levels");

    mapMsg["values"] = rmsValues;
    msg::post(mapMsg);
}


/// Sets the reporting interval in nanoseconds. 
void AudioLevel::interval(unsigned long long newInterval)
{
    assert(newInterval > 0);
    g_object_set (G_OBJECT(level_), "interval", newInterval, "message", emitMessages_, NULL);
}

