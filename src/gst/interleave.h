// interleave.h
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

#ifndef _INTERLEAVE_H_
#define _INTERLEAVE_H_

#include "gstBase.h"
#include <gst/audio/multichannel.h>

class AudioConfig;

class Interleave
    : public GstBase
{
public:
    Interleave(const AudioConfig &config);
    ~Interleave();
    void init();
    void link_to_sink(GstElement *sink);
    void link_to_src_vector(std::vector<GstElement *> &sources);

private:
    GstElement *interleave_;
    const AudioConfig &config_;
    static const GstAudioChannelPosition VORBIS_CHANNEL_POSITIONS[][8];
    void set_channel_layout();

    Interleave(const Interleave&);         //No Copy Constructor
    Interleave& operator=(const Interleave&);         //No Assignment Operator
};

#endif //_INTERLEAVE_H_

