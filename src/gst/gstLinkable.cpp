
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

#include "gstLinkable.h"
#include "logWriter.h"

void GstLinkable::link(std::vector<GstElement*> &sources, std::vector<GstElement*> &sinks)
{
    GstIter src;
    GstIter sink;
    for (src = sources.begin(), sink = sinks.begin(); src != sources.end(), sink != sinks.end();
         ++src, ++sink)
        assert(gst_element_link(*src, *sink));
}


void GstLinkable::link(GstElement *src, GstElement *sink)
{
    assert(gst_element_link(src, sink));
}


void GstLinkable::link(GstLinkableSource &src, GstElement *sink)
{
    GstElement *srcElement = src.srcElement();
    //FIXME: this is a hack to deal with leaf classes that don't implement srcElement
    //and/or sinkElement
    if (srcElement)
        assert(gst_element_link(srcElement, sink));
}


void GstLinkable::link(GstElement *src, GstLinkableSink &sink)
{
    assert(gst_element_link(src, sink.sinkElement()));
}


void GstLinkable::link(GstLinkableSource &src, GstLinkableSink &sink)
{
    GstElement *srcElement = src.srcElement();
    GstElement *sinkElement = sink.sinkElement();

    //FIXME: this is a hack to deal with leaf classes that don't implement srcElement
    //and/or sinkElement
    if (srcElement && sinkElement)
        assert(gst_element_link(srcElement, sinkElement));
}


void GstLinkable::link(std::vector<GstElement*> &sources, GstLinkableSink &sink)
{
    GstIter src;
    for (src = sources.begin(); src != sources.end(); ++src)
        assert(gst_element_link(*src, sink.sinkElement()));
}


void GstLinkable::link(GstLinkableSource &src, std::vector<GstElement*> &sinks)
{
    GstIter sink;
    for (sink = sinks.begin(); sink != sinks.end(); ++sink)
        assert(gst_element_link(src.srcElement(), *sink));
}


// with this method, we can find out why pads don't link
// if they fail
bool GstLinkable::link_pads(GstPad *srcPad, GstPad *sinkPad)
{
    bool linkOk = false;

    switch(gst_pad_link(srcPad, sinkPad))
    {
        case GST_PAD_LINK_OK:
            linkOk = true;
            break;

        case GST_PAD_LINK_WRONG_HIERARCHY:
            THROW_ERROR("pads have no common grandparent");
            break;

        case GST_PAD_LINK_WAS_LINKED:
            THROW_ERROR("pad was already linked");
            break;

        case GST_PAD_LINK_WRONG_DIRECTION:
            THROW_ERROR("pads have wrong direction");
            break;

        case GST_PAD_LINK_NOFORMAT:
            THROW_ERROR("pads do not have common format. Check if sample rates match.");
            break;

        case GST_PAD_LINK_NOSCHED:
            THROW_ERROR("pads cannot cooperate in scheduling");
            break;

        case GST_PAD_LINK_REFUSED:
            THROW_ERROR("refused for some reason");
            break;

        default:
            break;
    }

    return linkOk;
}


