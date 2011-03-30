/* gstBase.cpp
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

#include <gst/gst.h>
#include "gstLinkable.h"

void gstlinkable::link(std::vector<GstElement*> &sources, std::vector<GstElement*> &sinks)
{
    GstIter src;
    GstIter sink;
    for (src = sources.begin(), sink = sinks.begin();
            src != sources.end(), sink != sinks.end();
            ++src, ++sink)
        gstlinkable::link(*src, *sink);
}

void gstlinkable::link(GstElement *src, GstElement *sink)
{
    tryLink(src, sink);
}

// with this method, we can find out why pads don't link
// if they fail
bool gstlinkable::link_pads(GstPad *srcPad, GstPad *sinkPad)
{
    bool linkOk = false;

    switch(gst_pad_link(srcPad, sinkPad))
    {
        case GST_PAD_LINK_OK:
            linkOk = true;
            break;

        case GST_PAD_LINK_WRONG_HIERARCHY:
            THROW_CRITICAL("pads have no common grandparent");
            break;

        case GST_PAD_LINK_WAS_LINKED:
            THROW_CRITICAL("pad was already linked");
            break;

        case GST_PAD_LINK_WRONG_DIRECTION:
            THROW_CRITICAL("pads have wrong direction");
            break;

        case GST_PAD_LINK_NOFORMAT:
            THROW_CRITICAL("pads do not have common format. Check if sample rates match.");
            break;

        case GST_PAD_LINK_NOSCHED:
            THROW_CRITICAL("pads cannot cooperate in scheduling");
            break;

        case GST_PAD_LINK_REFUSED:
            THROW_CRITICAL("refused for some reason");
            break;

        default:
            break;
    }

    return linkOk;
}

