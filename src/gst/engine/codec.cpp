/* codec.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
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

#include <algorithm> // for std::find
#include <unistd.h>

#include <gst/gst.h>
#include <gst/audio/multichannel.h>

#include "util.h"

#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "codec.h"
#include "rtpPay.h"
#include "pipeline.h"
#include "mapMsg.h"

#include "rtpReceiver.h"

/// Constructor 
Encoder::Encoder() : encoder_(0)
{}


/// Destructor 
Encoder::~Encoder()
{
    Pipeline::Instance()->remove(&encoder_);
}

/// Returns bitrate property for this encoder
int Encoder::getBitrate() const
{
    tassert(encoder_);
    int bitrate; 
    g_object_get(G_OBJECT(encoder_), "bitrate", &bitrate, NULL);
    return bitrate;
}

/// Sets bitrate property for this encoder
void Encoder::setBitrate(int bitrate)
{
    tassert(encoder_);
    // if pipeline is playing, we need to set it to ready to make 
    // the bitrate change actually take effect
    if (Pipeline::Instance()->isPlaying())
    {
        Pipeline::Instance()->makeReady();
        g_object_set(G_OBJECT(encoder_), "bitrate", bitrate, NULL);
        Pipeline::Instance()->start();
    }
    else
    {
        LOG_DEBUG("SETTING BITRATE TO " << bitrate);
        g_object_set(G_OBJECT(encoder_), "bitrate", bitrate, NULL);
    }
}

/// Constructor 
Decoder::Decoder() : decoder_(0)
{}


/// Destructor 
Decoder::~Decoder()
{
    Pipeline::Instance()->remove(&decoder_);
}


VideoEncoder::VideoEncoder(GstElement *encoder, bool supportsInterlaced) :
    colorspc_(Pipeline::Instance()->makeElement("ffmpegcolorspace", NULL)), 
    supportsInterlaced_(supportsInterlaced)  // most codecs don't have this property
{
    encoder_ = encoder;
    tassert(encoder_);
    if (supportsInterlaced_)  // not all encoders have this property
        g_object_set(encoder_, "interlaced", TRUE, NULL); // true if we are going to encode interlaced material

    gstlinkable::link(colorspc_, encoder_);
}

/// Destructor 
VideoEncoder::~VideoEncoder()
{
    Pipeline::Instance()->remove(&colorspc_);
}

VideoDecoder::VideoDecoder() : doDeinterlace_(false), colorspc_(0), deinterlace_(0)
{}


/// Destructor 
VideoDecoder::~VideoDecoder()
{
    Pipeline::Instance()->remove(&colorspc_);
    Pipeline::Instance()->remove(&deinterlace_);
}


/// Sets up either decoder->queue->colorspace->deinterlace
/// or just decoder->queue
void VideoDecoder::init()
{
    // FIXME: should maybe be settable
    enum {ALL = 0, TOP, BOTTOM}; // deinterlace produces all fields, or top, bottom

    tassert(decoder_ != 0);
    if (doDeinterlace_)
    {
        colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", NULL); 
        LOG_DEBUG("DO THE DEINTERLACE");
        deinterlace_ = Pipeline::Instance()->makeElement("deinterlace", NULL);
        g_object_set(deinterlace_, "fields", TOP, NULL);
        gstlinkable::link(decoder_, colorspc_);
        gstlinkable::link(colorspc_, deinterlace_);
    }
}


/// Increase jitterbuffer size
void VideoDecoder::adjustJitterBuffer() 
{
    if (doDeinterlace_)
        RtpReceiver::setLatency(LONGER_JITTER_BUFFER_MS);
}



/// Constructor 
// POSIX specific hardware thread info 
// http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
// int numThreads = sysconf(_SC_NPROCESSORS_ONLN);

H264Encoder::H264Encoder(MapMsg &settings) : 
    VideoEncoder(Pipeline::Instance()->makeElement("x264enc", NULL), true),
    bitrate_(settings["bitrate"]) 
{
    // hardware threads: 1-n, 0 for automatic 
    int numThreads = boost::thread::hardware_concurrency();

    // numthreads should be 2 or 1.
    if (numThreads > 3) // don't hog all the cores
        numThreads = 3;
    else if (numThreads == 0)
        numThreads = 1;

    LOG_DEBUG("Using " << numThreads << " threads");
    g_object_set(encoder_, "threads", numThreads, NULL);
    // See gst-plugins-good/tests/examples/rtp/*h264*.sh
    g_object_set(encoder_, "byte-stream", TRUE, NULL);  

    // subme: subpixel motion estimation 1=fast, 6=best

    setBitrate(bitrate_);
}


/// Destructor 
H264Encoder::~H264Encoder()
{}

void Encoder::setBitrateInKbs(int newBitrate)
{
    static const double KB_PER_BIT = 0.001;
    Encoder::setBitrate(newBitrate * KB_PER_BIT);
}

/// Overridden to convert from bit/s to kbit/s
void H264Encoder::setBitrate(int newBitrate)
{
    setBitrateInKbs(newBitrate);
}


/// Creates an h.264 rtp payloader 
Pay* H264Encoder::createPayloader() const
{
    return new H264Pay();
}


void H264Decoder::init()
{
    decoder_ = Pipeline::Instance()->makeElement("ffdec_h264", NULL);
    VideoDecoder::init();
}


/// Creates an h.264 RtpDepay 
RtpPay* H264Decoder::createDepayloader() const
{
    return new H264Depay();
}


/// Increase jitterbuffer size
void H264Decoder::adjustJitterBuffer() 
{
    RtpReceiver::setLatency(LONGER_JITTER_BUFFER_MS);
}



/// Constructor 
H263Encoder::H263Encoder(MapMsg &settings) : 
    VideoEncoder(Pipeline::Instance()->makeElement("ffenc_h263p", NULL), false),
    bitrate_(settings["bitrate"])
{
    setBitrate(bitrate_);
}


/// Destructor 
H263Encoder::~H263Encoder()
{}


/// Creates an h.263 rtp payloader 
Pay* H263Encoder::createPayloader() const
{
    return new H263Pay();
}


void H263Decoder::init()
{
    decoder_ = Pipeline::Instance()->makeElement("ffdec_h263", NULL);
    VideoDecoder::init();
}


/// Creates an h.263 RtpDepay 
RtpPay* H263Decoder::createDepayloader() const
{
    return new H263Depay();
}


/// Constructor 
Mpeg4Encoder::Mpeg4Encoder(MapMsg &settings) : 
    VideoEncoder(Pipeline::Instance()->makeElement("ffenc_mpeg4", NULL), false), // FIXME: interlaced may cause stuttering
    bitrate_(settings["bitrate"])
{
    setBitrate(bitrate_);
}


/// Destructor 
Mpeg4Encoder::~Mpeg4Encoder()
{}


/// Creates an h.264 rtp payloader 
Pay* Mpeg4Encoder::createPayloader() const
{
    return new Mpeg4Pay();
}


void Mpeg4Decoder::init()
{
    decoder_ = Pipeline::Instance()->makeElement("ffdec_mpeg4", NULL);
    VideoDecoder::init();
}


/// Creates an mpeg4 RtpDepay 
RtpPay* Mpeg4Decoder::createDepayloader() const
{
    return new Mpeg4Depay();
}


/// Constructor 
TheoraEncoder::TheoraEncoder(MapMsg &settings) : 
    VideoEncoder(Pipeline::Instance()->makeElement("theoraenc", NULL), false),
    bitrate_(settings["bitrate"]), 
    quality_(settings["quality"]) 
{
    setSpeedLevel(MAX_SPEED_LEVEL);
    //g_object_set(encoder_, "keyframe-force", 1, NULL);
    if (bitrate_)
        setBitrate(bitrate_);
    else
        setQuality(quality_);
}


/// Destructor 
TheoraEncoder::~TheoraEncoder()
{}


/// Overridden to convert from bit/s to kbit/s
void TheoraEncoder::setBitrate(int newBitrate)
{
    LOG_DEBUG("Bitrate " << newBitrate);
    Encoder::setBitrateInKbs(newBitrate);
}


// theora specific
void TheoraEncoder::setQuality(int quality)
{
    tassert(encoder_ != 0);
    if (quality < MIN_QUALITY or quality > MAX_QUALITY)
        THROW_ERROR("Quality must be in range [" << MIN_QUALITY << "-" << MAX_QUALITY << "]");
    LOG_DEBUG("Quality " << quality);
    g_object_set(encoder_, "quality", quality, NULL);
}


// theora specific
void TheoraEncoder::setSpeedLevel(int speedLevel)
{
    tassert(encoder_ != 0);
    if (speedLevel < MIN_SPEED_LEVEL or speedLevel > MAX_SPEED_LEVEL)
        THROW_ERROR("Speed-level must be in range [" << MIN_SPEED_LEVEL << "-" << MAX_SPEED_LEVEL << "]");
    g_object_set(encoder_, "speed-level", speedLevel, NULL);
}


Pay* TheoraEncoder::createPayloader() const
{
    return new TheoraPay();
}


void TheoraDecoder::init()
{
    decoder_ = Pipeline::Instance()->makeElement("theoradec", NULL);
    VideoDecoder::init();
}

RtpPay* TheoraDecoder::createDepayloader() const
{
    return new TheoraDepay();
}

/// Constructor 
VorbisEncoder::VorbisEncoder() 
{
    encoder_ = Pipeline::Instance()->makeElement("vorbisenc", NULL);
}


/// Destructor 
VorbisEncoder::~VorbisEncoder() 
{}

/// Creates an RtpVorbisPay 
Pay* VorbisEncoder::createPayloader() const
{
    return new VorbisPay();
}


unsigned long long VorbisDecoder::minimumBufferTime()
{
    return MIN_BUFFER_USEC;
}


VorbisDecoder::VorbisDecoder()
{
    decoder_ = Pipeline::Instance()->makeElement("vorbisdec", NULL);
}


/// Creates an RtpVorbisDepay 
RtpPay* VorbisDecoder::createDepayloader() const
{
    return new VorbisDepay();
}

/// Constructor
RawEncoder::RawEncoder() : 
    aconv_(Pipeline::Instance()->makeElement("audioconvert", NULL))
{}

/// Destructor
RawEncoder::~RawEncoder()
{
    Pipeline::Instance()->remove(&aconv_);
}

/// Creates an RtpL16Pay 
Pay* RawEncoder::createPayloader() const
{
    return new L16Pay();
}

/// Constructor
RawDecoder::RawDecoder() :
    aconv_(Pipeline::Instance()->makeElement("audioconvert", NULL))
{}


/// Destructor
RawDecoder::~RawDecoder()
{
    Pipeline::Instance()->remove(&aconv_);
}

/// Creates an RtpL16Depay 
RtpPay* RawDecoder::createDepayloader() const
{
    return new L16Depay();
}

/// Constructor
LameEncoder::LameEncoder() : 
    aconv_(Pipeline::Instance()->makeElement("audioconvert", NULL)),
    mp3parse_(Pipeline::Instance()->makeElement("mp3parse", NULL))
{
    /// FIXME: put this in initializer list somehow
    encoder_ = Pipeline::Instance()->makeElement("lamemp3enc", NULL);
    gstlinkable::link(aconv_, encoder_);
    gstlinkable::link(encoder_, mp3parse_);
}


/// Destructor
LameEncoder::~LameEncoder()
{
    Pipeline::Instance()->remove(&mp3parse_);
    Pipeline::Instance()->remove(&aconv_);
}

/// Constructor
MadDecoder::MadDecoder() :
    aconv_(Pipeline::Instance()->makeElement("audioconvert", NULL))
{
    decoder_ = Pipeline::Instance()->makeElement("mad", NULL);
    gstlinkable::link(decoder_, aconv_);
}

MadDecoder::~MadDecoder()
{
    Pipeline::Instance()->remove(&aconv_);
}


/** 
 * Creates an RtpMpaPay */
Pay* LameEncoder::createPayloader() const
{
    return new MpaPay();
}

/// Creates an RtpMpaDepay 
RtpPay* MadDecoder::createDepayloader() const
{
    return new MpaDepay();
}

