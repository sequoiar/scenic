
// gstBase.cpp
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
#include "gstBase.h"
#include "pipeline.h"
#include "logWriter.h"

int GstBase::refCount_ = 0; 
const unsigned int GstBase::SAMPLE_RATE = 48000;

// this initializes pipeline only once/process
GstBase::GstBase() : pipeline_(Pipeline::Instance())
{ 
    ++refCount_; 
}

GstBase::~GstBase()
{
    stop();
    --refCount_;
    if (refCount_ <= 0)
    {
        assert(refCount_ == 0);
        pipeline_.reset();
    }
}


void GstBase::start()
{
    pipeline_.start();
}


void GstBase::pause()
{
    pipeline_.pause();
}


void GstBase::stop()
{
    pipeline_.stop();
}


bool GstBase::isPaused() const
{ 
    return pipeline_.isPaused(); 
}


bool GstBase::isPlaying() const
{ 
    return pipeline_.isPlaying(); 
}


const char* GstBase::getElementPadCaps(GstElement *element, const char * padName) const
{
    assert(isPlaying() || isPaused());

    GstPad *pad;
    GstCaps *caps;

    assert(pad = gst_element_get_pad(GST_ELEMENT(element), padName));

    do
        caps = gst_pad_get_negotiated_caps(pad);
    while (caps == NULL);

    // goes until caps are initialized

    gst_object_unref(pad);

    const char *result = gst_caps_to_string(caps);
    gst_caps_unref(caps);
    return result;
}


void GstBase::checkCapsSampleRate(GstCaps *caps)
{
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    const GValue *value  = gst_structure_get_value(structure, "clock-rate");

    if (GstBase::SAMPLE_RATE != static_cast<unsigned>(g_value_get_int(value)))
        THROW_CRITICAL("Sample rate " << g_value_get_int(value) << 
                " of caps does not match sample rate " << 
                GstBase::SAMPLE_RATE << " of pipeline.");
}

