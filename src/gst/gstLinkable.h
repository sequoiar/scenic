
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

class GstLinkable 
    : public GstBase
{
    public:
        static bool link_pads(GstPad *srcPad, GstPad *sinkPad);
        static void link(std::vector<GstElement*> &sources, std::vector<GstElement*> &sinks);
        static void link(GstElement *src, GstElement *sink);
        static void link(GstLinkable &src, GstElement *sink);
        static void link(GstElement *src, GstLinkable &sink);
        static void link(GstLinkable &src, GstLinkable &sink);
        static void link(std::vector<GstElement*> &sources, GstLinkable &sink);
        static void link(GstLinkable &source, std::vector<GstElement*> &sinks);
        
    protected:

        GstLinkable() {};
        ~GstLinkable() {};

        virtual GstElement *element() = 0;
};

#endif // _GST_LINKABLE_H_

