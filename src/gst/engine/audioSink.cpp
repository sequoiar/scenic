// audioSink.cpp
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
#include <gst/gst.h>
#include "audioSink.h"
#include "logWriter.h"
#include "jackUtils.h"
#include "pipeline.h"
#include "alsa.h"

AudioSink::~AudioSink()
{
    Pipeline::Instance()->remove(&sink_);
}


std::string AudioSink::getCaps()
{
    return Pipeline::Instance()->getElementPadCaps(sink_, "sink");
}


void AudioJackSink::init()
{
    if (!Jack::is_running())
        THROW_CRITICAL("Jack is not running");

    sink_ = Pipeline::Instance()->makeElement("jackaudiosink", NULL);
    g_object_set(G_OBJECT(sink_), "connect", 1, NULL);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

    if (Pipeline::SAMPLE_RATE != Jack::samplerate())
        THROW_CRITICAL("Jack's sample rate of " << Jack::samplerate()
                << " does not match default sample rate " << Pipeline::SAMPLE_RATE);
}

        
AudioAlsaSink::~AudioAlsaSink()
{
    Pipeline::Instance()->remove(&audioconvert_);
}

void AudioAlsaSink::init()
{
    if (Jack::is_running())
        THROW_CRITICAL("Jack is running, you must stop jack server before using alsasink");

    audioconvert_ = Pipeline::Instance()->makeElement("audioconvert", NULL);

    sink_ = Pipeline::Instance()->makeElement("alsasink", NULL);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "device", Alsa::DEVICE_NAME, NULL);

    gstlinkable::link(audioconvert_, sink_);
}


AudioPulseSink::~AudioPulseSink()
{
    Pipeline::Instance()->remove(&audioconvert_);
}

void AudioPulseSink::init()
{
    audioconvert_ = Pipeline::Instance()->makeElement("audioconvert", NULL);

    sink_ = Pipeline::Instance()->makeElement("pulsesink", NULL);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);

    gstlinkable::link(audioconvert_, sink_);
}

