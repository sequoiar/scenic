// codec.cpp
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


#include <cassert>
#include "codec.h"


Codec::~Codec()
{
    assert(stop());
    pipeline_.remove(&codec_);
}


H264Encoder::~H264Encoder()
{
    assert(stop());
    pipeline_.remove(&colorspc_);
}


bool H264Encoder::init()
{
    assert(colorspc_ = gst_element_factory_make("ffmpegcolorspace", "colorspc"));
    pipeline_.add(colorspc_);

    assert(codec_ = gst_element_factory_make("x264enc", NULL));
    g_object_set(G_OBJECT(codec_), "bitrate", 2048, "byte-stream", TRUE, "threads", 4,
                     NULL);
    pipeline_.add(codec_);

    GstLinkable::link(colorspc_, codec_);
    return true;
}


bool H264Decoder::init()
{
    assert(codec_ = gst_element_factory_make("ffdec_h264", NULL));
    pipeline_.add(codec_);
    return true;
}


bool VorbisEncoder::init()
{
    assert(codec_ = gst_element_factory_make("vorbisenc", NULL));
    pipeline_.add(codec_);
    return true;
}


bool VorbisDecoder::init()
{
    assert(codec_ = gst_element_factory_make("vorbisdec", NULL));
    pipeline_.add(codec_);
    return true;
}


