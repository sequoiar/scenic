// dv1394.h
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

#ifndef _DV1394_H_
#define _DV1394_H_

#include "noncopyable.h"

class Pipeline;
class _GstElement;
class _GstPad;

class Dv1394 : private boost::noncopyable
{
    public:
        /// This is the single point of access to the singleton instance of this Dv1394 object
        static Dv1394 * Instance(const Pipeline &pipeline);
        static void reset();
        void setAudioSink(_GstElement *audioSink);
        void setVideoSink(_GstElement *videoSink);
        void unsetAudioSink();
        void unsetVideoSink();
        static void cb_new_src_pad(_GstElement *  /*srcElement*/, _GstPad * srcPad, void *data);
        // FIXME: SO GROSS!
        void doTimestamp();

    private:
        Dv1394(const Pipeline &pipeline);
        const Pipeline &pipeline_;

        _GstElement *dv1394src_;
        _GstElement *dvdemux_;
        _GstElement *audioSink_;
        _GstElement *videoSink_;
        static Dv1394 *instance_;
};

#endif //_DV1394_H_

