/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cassert>

#include <gst/gst.h>
#include <gst_linkable.h>
#include "audio_sink.h"
#include "audio_config.h"
#include "jack_util.h"
#include "pipeline.h"

/// Constructor
AudioSink::AudioSink() :
    sink_(0)
{
}

/// Destructor
AudioSink::~AudioSink()
{
}

void AudioSink::adjustBufferTime(unsigned long long bufferTime)
{
    g_object_set(sink_, "buffer-time", bufferTime, NULL);
    unsigned long long val;
    g_object_get(sink_, "buffer-time", &val, NULL);
    LOG_DEBUG("Buffer time is " << val);
}

/// Constructor
AudioAlsaSink::AudioAlsaSink(Pipeline &pipeline, const AudioSinkConfig &config) :
    aconv_(pipeline.makeElement("audioconvert", NULL)),
    config_(config)
{
    sink_ = pipeline.makeElement("alsasink", NULL);

    g_object_set(G_OBJECT(sink_), "buffer-time", config_.bufferTime(), NULL);
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(sink_), "device", config_.deviceName(), NULL);

    gstlinkable::link(aconv_, sink_);
}

/// Constructor
AudioPulseSink::AudioPulseSink(Pipeline &pipeline, const AudioSinkConfig &config) :
    aconv_(pipeline.makeElement("audioconvert", NULL)),
    config_(config)
{
    sink_ = pipeline.makeElement("pulsesink", NULL);
    g_object_set(G_OBJECT(sink_), "buffer-time", config_.bufferTime(), NULL);
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(sink_), "device", config_.deviceName(), NULL);

    gstlinkable::link(aconv_, sink_);
}


/// Constructor
AudioAutoSink::AudioAutoSink(Pipeline &pipeline, const AudioSinkConfig &config) :
    aconv_(pipeline.makeElement("audioconvert", NULL)),
    config_(config)
{
    sink_ = pipeline.makeElement("autoaudiosink", NULL);

    g_object_set(G_OBJECT(sink_), "buffer-time", config_.bufferTime(), NULL);
    if (config_.hasDeviceName())
        g_object_set(G_OBJECT(sink_), "device", config_.deviceName(), NULL);

    gstlinkable::link(aconv_, sink_);
}

/// Constructor
AudioJackSink::AudioJackSink(Pipeline &pipeline, const AudioSinkConfig &config) :
    config_(config)
{
    sink_ = pipeline.makeElement("jackaudiosink", config_.sinkName());

    // uncomment to turn off autoconnect
    //g_object_set(G_OBJECT(sink_), "connect", 0, NULL);
    // use auto-forced connect mode if available
    g_object_set(G_OBJECT(sink_), "connect", 2, NULL);

    if (config_.bufferTime() < Jack::safeBufferTime())
    {
        LOG_WARNING("Buffer time " << config_.bufferTime() << " is too low, using " << Jack::safeBufferTime() << " instead");
        g_object_set(G_OBJECT(sink_), "buffer-time", Jack::safeBufferTime(), NULL);
    }
    else
        g_object_set(G_OBJECT(sink_), "buffer-time", config_.bufferTime(), NULL);

    unsigned long long val;
    g_object_get(sink_, "buffer-time", &val, NULL);
    LOG_DEBUG("Buffer time is " << val);
}

void AudioJackSink::disableAutoConnect()
{
    assert(sink_);
    g_object_set(G_OBJECT(sink_), "connect", 0, NULL);
}

