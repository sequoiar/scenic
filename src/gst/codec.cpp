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

#include <gst/gst.h>
#include <cassert>
#include <gst/audio/multichannel.h>
#include "codec.h"
#include "rtpPay.h"
#include "pipeline.h"
#include "logWriter.h"


Codec::~Codec()
{
    stop();
    pipeline_.remove(&codec_);
}


H264Encoder::~H264Encoder()
{
    stop();
    pipeline_.remove(&colorspc_);
}


void H264Encoder::init()
{
    assert(colorspc_ = gst_element_factory_make("ffmpegcolorspace", "colorspc"));
    pipeline_.add(colorspc_);

    assert(codec_ = gst_element_factory_make("x264enc", NULL));
    g_object_set(G_OBJECT(codec_), "bitrate", 4096, "byte-stream", TRUE, "threads", 4,
                     NULL);
    pipeline_.add(codec_);

    GstLinkable::link(colorspc_, codec_);
}


RtpPay* H264Encoder::createPayloader() const
{
    return new H264Payloader();
}


void H264Decoder::init()
{
    assert(codec_ = gst_element_factory_make("ffdec_h264", NULL));
    pipeline_.add(codec_);
}


RtpPay* H264Decoder::createDepayloader() const
{
    return new H264Depayloader();
}


void VorbisEncoder::init()
{
    assert(codec_ = gst_element_factory_make("vorbisenc", NULL));
    pipeline_.add(codec_);
}


void VorbisDecoder::init()
{
    assert(codec_ = gst_element_factory_make("vorbisdec", NULL));
    pipeline_.add(codec_);
}


RtpPay* VorbisEncoder::createPayloader() const
{
    return new VorbisPayloader();
}


RtpPay* VorbisDecoder::createDepayloader() const
{
    return new VorbisDepayloader();
}


void RawEncoder::init()
{
    assert(aconv_ = gst_element_factory_make("audioconvert", NULL));
    pipeline_.add(aconv_);
}


void RawDecoder::init()
{
    assert(aconv_ = gst_element_factory_make("audioconvert", NULL));
    pipeline_.add(aconv_);
}


RtpPay* RawEncoder::createPayloader() const
{
    return new L16Payloader();
}


RtpPay* RawDecoder::createDepayloader() const
{
    return new L16Depayloader();
}

RawEncoder::~RawEncoder()
{
    stop();
    pipeline_.remove(&aconv_);
}


RawDecoder::~RawDecoder()
{
    stop();
    pipeline_.remove(&aconv_);
}


void LameEncoder::init()
{
    assert(aconv_ = gst_element_factory_make("audioconvert", NULL));
    pipeline_.add(aconv_);
    assert(codec_ = gst_element_factory_make("lame", NULL));
    pipeline_.add(codec_);

    GstLinkable::link(aconv_, codec_);
}


void MadDecoder::init()
{
    assert(codec_ = gst_element_factory_make("mad", NULL));
    pipeline_.add(codec_);
    assert(aconv_ = gst_element_factory_make("audioconvert", NULL));
    pipeline_.add(aconv_);

    GstLinkable::link(codec_, aconv_);
}


RtpPay* LameEncoder::createPayloader() const
{
    return new MpaPayloader();
}


RtpPay* MadDecoder::createDepayloader() const
{
    return new MpaDepayloader();
}

LameEncoder::~LameEncoder()
{
    stop();
    pipeline_.remove(&aconv_);
}


MadDecoder::~MadDecoder()
{
    stop();
    pipeline_.remove(&aconv_);
}

