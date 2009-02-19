// interleave.h
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

#ifndef _INTERLEAVE_H_
#define _INTERLEAVE_H_

#include <gst/audio/multichannel.h>
#include "gstLinkable.h"

class AudioSourceConfig;

class Interleave
    : public GstLinkableFilter
{
    private:
        /** The interleave functionality used to be part of the same class
        * as InterleavedAudioSource. When subdividing one class into two
        * separate classes, it make sense for them to be friends. Also
        * InterleavedAudioSource's internals are safe
        * as InterleavedAudioSource's children will not have access here. */
        friend class InterleavedAudioSource;
        explicit Interleave(const AudioSourceConfig &config)
            : interleave_(0), config_(config) {}
        ~Interleave();
        void init();

        GstElement *srcElement() { return interleave_; }
        GstElement *sinkElement() { return interleave_; }

        GstElement *interleave_;
        const AudioSourceConfig &config_;
        static const GstAudioChannelPosition VORBIS_CHANNEL_POSITIONS[][8];
        void set_channel_layout();

        Interleave(const Interleave&);     //No Copy Constructor
        Interleave& operator=(const Interleave&);     //No Assignment Operator
};

#endif //_INTERLEAVE_H_

