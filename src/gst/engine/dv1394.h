// dv1394.h
// Copyright (C) 2009 Société des arts technologiques (SAT)
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

#ifndef _DV1394_H_
#define _DV1394_H_

#include "gstLinkable.h"


class _GstElement;
class _GstPad;

class Dv1394
{
    public:
        /// This is the single point of access to the singleton instance of this Dv1394 object
        static Dv1394 * Instance();
        static void reset();
        void setAudioSink(_GstElement *audioSink);
        void setVideoSink(_GstElement *videoSink);
        void unsetAudioSink();
        void unsetVideoSink();
        static void cb_new_src_pad(_GstElement *  /*srcElement*/, _GstPad * srcPad, void *data);

    private:
        explicit Dv1394() : dv1394src_(0), dvdemux_(0), audioSink_(0), videoSink_(0) {}
        ~Dv1394();
        void init();

        _GstElement *dv1394src_;
        _GstElement *dvdemux_;
        _GstElement *audioSink_;
        _GstElement *videoSink_;
        static Dv1394 *instance_;

        Dv1394(const Dv1394&);     //No Copy Constructor
        Dv1394& operator=(const Dv1394&);     //No Assignment Operator
};

#endif //_DV1394_H_

