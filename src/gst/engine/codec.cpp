/* codec.cpp
 * Copyright (C) 2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include <gst/gst.h>
#include <algorithm> // for std::find
#include <gst/audio/multichannel.h>
#include "codec.h"
#include "rtpPay.h"
#include "pipeline.h"
#include "mapMsg.h"


const std::string Codec::VALID_CODECS[NUM_CODECS] = {"h264", "raw", "vorbis", "mp3", "mpeg4", "h263"};

/// Constructor 
Codec::Codec() : 
    codec_(0) 
{}


/// Destructor 
Codec::~Codec()
{
    Pipeline::Instance()->remove(&codec_);
}


/// Returns true if the specified codec is supported by our architecture
bool Codec::isSupportedCodec(const std::string & codecStr)
{
    if (codecStr.empty())
    {
        LOG_WARNING("Empty codec string given");
        return false;
    }

    // ptr to one past the end of our array of codecs
    const std::string *VALID_CODECS_END = VALID_CODECS + (NUM_CODECS * sizeof(std::string));
    // search for codecStr in Codec's list of supported codecs
    return std::find(VALID_CODECS, VALID_CODECS_END, codecStr) != VALID_CODECS_END;
}


/// Returns bitrate property for this encoder
int Encoder::getBitrate()
{
    assert(codec_);
    unsigned bitrate; 
    g_object_get(G_OBJECT(codec_), "bitrate", &bitrate, NULL);
    return bitrate;
}

/// Sets bitrate property for this encoder
void Encoder::setBitrate(unsigned bitrate)
{
    assert(codec_);
    // if pipeline is playing, we need to set it to ready to make 
    // the bitrate change actually take effect
    if (Pipeline::Instance()->isPlaying())
    {
        Pipeline::Instance()->makeReady();
        g_object_set(G_OBJECT(codec_), "bitrate", bitrate, NULL);
        Pipeline::Instance()->start();
    }
    else
        g_object_set(G_OBJECT(codec_), "bitrate", bitrate, NULL);
}


/// Posts bitrate using MapMsg
void Encoder::postBitrate()
{
    assert(codec_);
    MapMsg mapMsg("bitrate");
    std::string codecName(gst_element_factory_get_longname(gst_element_get_factory(codec_)));
    std::stringstream msgStream; 
    msgStream << codecName << ": " << getBitrate();
    mapMsg["value"] = msgStream.str();
    msg::post(mapMsg);
}

/// Constructor 
AudioConvertedEncoder::AudioConvertedEncoder() : 
    aconv_(0) 
{}

void AudioConvertedEncoder::init()
{
    aconv_ = Pipeline::Instance()->makeElement("audioconvert", NULL);
}

/// Destructor 
AudioConvertedEncoder::~AudioConvertedEncoder()
{
    Pipeline::Instance()->remove(&aconv_);
}


/// Constructor 
AudioConvertedDecoder::AudioConvertedDecoder() : 
    aconv_(0) 
{}


void AudioConvertedDecoder::init()
{
    aconv_ = Pipeline::Instance()->makeElement("audioconvert", NULL);
}

/// Destructor 
AudioConvertedDecoder::~AudioConvertedDecoder()
{
    Pipeline::Instance()->remove(&aconv_);
}


/// Constructor 
H264Encoder::H264Encoder() : 
    colorspc_(0)
{}


/// Destructor 
H264Encoder::~H264Encoder()
{
    Pipeline::Instance()->remove(&colorspc_);
}


void H264Encoder::init()
{
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", "colorspc");

    codec_ = Pipeline::Instance()->makeElement("x264enc", NULL);
    // subme: subpixel motion estimation 1=fast, 6=best
    // threads: 1-4, 0 for automatic 

    gstlinkable::link(colorspc_, codec_);
}


/// Overridden to convert from bit/s to kbit/s
void H264Encoder::setBitrate(unsigned newBitrate)
{
    Encoder::setBitrate(newBitrate * 0.001);
}


/// Creates an h.264 rtp payloader 
RtpPay* H264Encoder::createPayloader() const
{
    return new H264Payloader();
}


void H264Decoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("ffdec_h264", NULL);
}


/// Creates an h.264 RtpDepayloader 
RtpPay* H264Decoder::createDepayloader() const
{
    return new H264Depayloader();
}


const char *H264Decoder::getCaps() const
{
    return "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264," 
    "profile-level-id=(string)4d4033, sprop-parameter-sets=(string)\"Z01AM5JUBgHtgIgAAB9IAAdTBHjBlQ\\=\\=\\,aO48gA\\=\\=\","
    "payload=(int)96";
}


/// Constructor 
H263Encoder::H263Encoder() : 
    colorspc_(0)
{}


/// Destructor 
H263Encoder::~H263Encoder()
{
    Pipeline::Instance()->remove(&colorspc_);
}


void H263Encoder::init()
{
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", "colorspc");

    codec_ = Pipeline::Instance()->makeElement("ffenc_h263", NULL);

    gstlinkable::link(colorspc_, codec_);
}


/// Creates an h.263 rtp payloader 
RtpPay* H263Encoder::createPayloader() const
{
    return new H263Payloader();
}


void H263Decoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("ffdec_h263", NULL);
}


/// Creates an h.263 RtpDepayloader 
RtpPay* H263Decoder::createDepayloader() const
{
    return new H263Depayloader();
}


/// These caps are the same for any h263 stream
const char *H263Decoder::getCaps() const
{
    return "application/x-rtp,media=(string)video,clock-rate=(int)90000,"
        "encoding-name=(string)H263,"
        "payload=(int)96";
}


/// Constructor 
Mpeg4Encoder::Mpeg4Encoder() : 
    colorspc_(0)
{}


void Mpeg4Encoder::init()
{
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", "colorspc");

    codec_ = Pipeline::Instance()->makeElement("ffenc_mpeg4", NULL);

    gstlinkable::link(colorspc_, codec_);
}


/// Creates an h.264 rtp payloader 
RtpPay* Mpeg4Encoder::createPayloader() const
{
    return new Mpeg4Payloader();
}


void Mpeg4Decoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("ffdec_mpeg4", NULL);
}


/// Creates an mpeg4 RtpDepayloader 
RtpPay* Mpeg4Decoder::createDepayloader() const
{
    return new Mpeg4Depayloader();
}


/** The config string will vary depending on resolution. Not an issue for NTSC only,
but for other resolutions this will lead to a picture melting, sensory experience.
*/
const char *Mpeg4Decoder::getCaps() const
{
    // FIXME: This sucks!!!!!! This should be sent to the receiver from the sender.
    return "application/x-rtp,media=(string)video,clock-rate=(int)90000,"
        "config=(string)000001b001000001b58913000001000000012000c48d8ba98518043c1463000001b24c61766335322e362e30,"
        "encoding-name=(string)MP4V-ES,"
        "profile-level-id=(string)1,"
        "payload=(int)96";
}


/// Constructor 
VorbisEncoder::VorbisEncoder() 
{}

void VorbisEncoder::init()
{
    AudioConvertedEncoder::init();
    codec_ = Pipeline::Instance()->makeElement("vorbisenc", NULL);
    gstlinkable::link(aconv_, codec_);
}


/// Creates an RtpVorbisPayloader 
RtpPay* VorbisEncoder::createPayloader() const
{
    return new VorbisPayloader();
}


void VorbisDecoder::init()
{
    codec_ = Pipeline::Instance()->makeElement("vorbisdec", NULL);
}

/// Creates an RtpVorbisDepayloader 
RtpPay* VorbisDecoder::createDepayloader() const
{
    return new VorbisDepayloader();
}

/// Constructor
RawEncoder::RawEncoder()
{}


void RawEncoder::init()
{
    AudioConvertedEncoder::init();
}

/// Creates an RtpL16Payloader 
RtpPay* RawEncoder::createPayloader() const
{
    return new L16Payloader();
}

/// Constructor
RawDecoder::RawDecoder()
{}


/// Creates an RtpL16Depayloader 
RtpPay* RawDecoder::createDepayloader() const
{
    return new L16Depayloader();
}

/// Constructor
LameEncoder::LameEncoder()
{}

void LameEncoder::init()
{
    AudioConvertedEncoder::init();
    codec_ = Pipeline::Instance()->makeElement("lame", NULL);
    gstlinkable::link(aconv_, codec_);
}


/// Constructor
MadDecoder::MadDecoder()
{}


void MadDecoder::init()
{
    AudioConvertedDecoder::init();
    codec_ = Pipeline::Instance()->makeElement("mad", NULL);
    gstlinkable::link(codec_, aconv_);
}

/** 
 * Creates an RtpMpaPayloader */
RtpPay* LameEncoder::createPayloader() const
{
    return new MpaPayloader();
}

/// Creates an RtpMpaDepayloader 
RtpPay* MadDecoder::createDepayloader() const
{
    return new MpaDepayloader();
}

