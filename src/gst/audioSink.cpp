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
#include "gstBase.h"
#include "logWriter.h"
#include "jackUtils.h"
#include "pipeline.h"


AudioSink::~AudioSink()
{
    stop();
    pipeline_.remove(&sink_);
}


std::string AudioSink::getCaps()
{
    return getElementPadCaps(sink_, "sink");
}


void AudioJackSink::init()
{
    if (!Jack::is_running())
        THROW_CRITICAL("Jack is not running");

    assert(sink_ = gst_element_factory_make("jackaudiosink", NULL));
    g_object_set(G_OBJECT(sink_), "connect", 1, NULL);
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    pipeline_.add(sink_);

    if (GstBase::SAMPLE_RATE != Jack::samplerate())
        THROW_CRITICAL("Jack's sample rate of " << Jack::samplerate()
                << " does not match default sample rate " << GstBase::SAMPLE_RATE);
}


void AudioAlsaSink::init()
{
    if (Jack::is_running())
        THROW_CRITICAL("Jack is running, stop jack server");

    assert(sink_ = gst_element_factory_make("alsasink", NULL));
    g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    pipeline_.add(sink_);
}


