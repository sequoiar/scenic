
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

#include <gst/gst.h>
#include <cassert>

#include "gstBase.h"
#include "logWriter.h"

int GstBase::refCount_ = 0;

// this initializes pipeline only once/process
GstBase::GstBase()
    : pipeline_(Pipeline::Instance())
{
    ++refCount_;
}


GstBase::~GstBase()
{
    if (isPlaying())
        assert(pipeline_.stop());
    --refCount_;
    if (refCount_ <= 0)
    {
        assert(refCount_ == 0);
        //pipeline_.reset();
    }
}


bool GstBase::isPlaying()
{
    return pipeline_.isPlaying();
}


// with this method, we can find out why pads don't link
// if they fail
bool GstBase::link_pads(GstPad *srcPad, GstPad *sinkPad)
{
    bool linkOk = false;

    switch(gst_pad_link(srcPad, sinkPad))
    {
        case GST_PAD_LINK_OK:
            LOG("link succeeded");
            linkOk = true;
            break;

        case GST_PAD_LINK_WRONG_HIERARCHY:
            LOG("pads have no common grandparent");
            break;

        case GST_PAD_LINK_WAS_LINKED:
            LOG("pad was already linked");
            break;

        case GST_PAD_LINK_WRONG_DIRECTION:
            LOG("pads have wrong direction");
            break;

        case GST_PAD_LINK_NOFORMAT:
            LOG("pads do not have common format");
            break;

        case GST_PAD_LINK_NOSCHED:
            LOG("pads cannot cooperate in scheduling");
            break;

        case GST_PAD_LINK_REFUSED:
            LOG("refused for some reason");
            break;

        default:
            break;
    }

    return linkOk;
}


void GstBase::link_element_vectors(std::vector<GstElement*> &sources,
                                   std::vector<GstElement*> &sinks)
{
    GstIter src;
    GstIter sink;
    for (src = sources.begin(), sink = sinks.begin(); src != sources.end(), sink != sinks.end();
         ++src, ++sink)
        assert(gst_element_link(*src, *sink));
}


