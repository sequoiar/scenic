
// gstLinkable.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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

#ifndef _GST_LINKABLE_H_
#define _GST_LINKABLE_H_

#include <vector> 
#include "except.h"

class GstLinkableSource;
class GstLinkableSink;
class _GstElement;
class _GstPad;

typedef std::vector<_GstElement *>::iterator GstIter;

namespace gstlinkable
{
    class LinkExcept : public Except
    {
        public:
            LinkExcept(const char* log_msg) : Except(log_msg)
        { 
            log_ = WARNING;
        }
    };

    bool link_pads(_GstPad *srcPad, _GstPad *sinkPad);
    void link(std::vector<_GstElement*> &sources, std::vector<_GstElement*> &sinks);
    void link(_GstElement *src, _GstElement *sink);
    void link(GstLinkableSource &src, _GstElement *sink);
    void link(_GstElement *src, GstLinkableSink &sink);
    void link(GstLinkableSource &src, GstLinkableSink &sink);
    void link(std::vector<_GstElement*> &sources, GstLinkableSink &sink);
    void link(GstLinkableSource &source, std::vector<_GstElement*> &sinks);
    void tryLink(_GstElement *src, _GstElement *sink);
}

class GstLinkableSource 
{
    public:
        GstLinkableSource() {} 
        virtual ~GstLinkableSource() {} 
        virtual _GstElement *srcElement() = 0;
};


class GstLinkableSink 
{
    public:
        GstLinkableSink() {}
        virtual ~GstLinkableSink() {} 
        virtual _GstElement *sinkElement() = 0;
};


class GstLinkableFilter
: public GstLinkableSource, public GstLinkableSink
{
    public:
        GstLinkableFilter() {} 
};

#endif // _GST_LINKABLE_H_

