
// gstLinkable.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _GST_LINKABLE_H_
#define _GST_LINKABLE_H_

#include <vector>
#include <gst/gst.h>
#include "except.h"
#include "util/log_writer.h"

class _GstPad;

namespace gstlinkable
{
    typedef std::vector<GstElement *>::iterator GstIter;
    class LinkExcept : public Except
    {
        public:
            LinkExcept(const char* log_msg) : Except(log_msg)
        {
            log_ = WARNING;
        }
    };


    static void tryLink(GstElement *src, GstElement *sink)
    {
        if (not gst_element_link(src, sink))
        {
            std::string msg("Failed to link ");
            msg += GST_ELEMENT_NAME(src);
            msg += " to ";
            msg += GST_ELEMENT_NAME(sink);
            throw LinkExcept(msg.c_str());
        }
    }

    bool link_pads(_GstPad *srcPad, _GstPad *sinkPad);
    void link(std::vector<GstElement*> &sources, std::vector<GstElement*> &sinks);
    void link(GstElement *src, GstElement *sink);

    template <typename T>
    void link(T &src, GstElement *sink)
    {
        tryLink(src.srcElement(), sink);
    }

    template <typename T>
    void link(GstElement *src, T &sink)
    {
        tryLink(src, sink.sinkElement());
    }

    template <typename T, typename U>
    void link(T &src, U &sink)
    {
        tryLink(src.srcElement(), sink.sinkElement());
    }

    template <typename T>
    void link(std::vector<GstElement*> &sources, T &sink)
    {
        GstIter src;
        for (src = sources.begin(); src != sources.end(); ++src)
            tryLink(*src, sink.sinkElement());
    }

    template <typename T>
    void link(T &source, std::vector<GstElement*> &sinks)
    {
        GstIter sink;
        for (sink = sinks.begin(); sink != sinks.end(); ++sink)
            tryLink(source.srcElement(), *sink);
    }
}

#endif // _GST_LINKABLE_H_

