
// gstLinkable.h
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

#ifndef _GST_LINKABLE_H_
#define _GST_LINKABLE_H_

#include "gstBase.h"

class GstLinkableSource;
class GstLinkableSink;
class _GstElement;
class _GstPad;

namespace GstLinkable
{
    bool link_pads(_GstPad *srcPad, _GstPad *sinkPad);
    void link(std::vector<_GstElement*> &sources, std::vector<_GstElement*> &sinks);
    void link(_GstElement *src, _GstElement *sink);
    void link(GstLinkableSource &src, _GstElement *sink);
    void link(_GstElement *src, GstLinkableSink &sink);
    void link(GstLinkableSource &src, GstLinkableSink &sink);
    void link(std::vector<_GstElement*> &sources, GstLinkableSink &sink);
    void link(GstLinkableSource &source, std::vector<_GstElement*> &sinks);
}

class GstLinkableSource
: virtual public GstBase
{
    public:
        GstLinkableSource() {} 
        virtual _GstElement *srcElement() = 0;
    private:
        GstLinkableSource(const GstLinkableSource&);     //No Copy Constructor
        GstLinkableSource& operator=(const GstLinkableSource&);     //No Assignment Operator
};


class GstLinkableSink
: virtual public GstBase
{
    public:
        GstLinkableSink() {}
        virtual _GstElement *sinkElement() = 0;
    private:
        GstLinkableSink(const GstLinkableSink&);     //No Copy Constructor
        GstLinkableSink& operator=(const GstLinkableSink&);     //No Assignment Operator
};


class GstLinkableFilter
: public GstLinkableSource, public GstLinkableSink
{
    public:
        GstLinkableFilter() {} 
    private:
        GstLinkableFilter(const GstLinkableFilter&);     //No Copy Constructor
        GstLinkableFilter& operator=(const GstLinkableFilter&);     //No Assignment Operator
};

#endif // _GST_LINKABLE_H_

