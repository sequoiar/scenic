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

#include "rtpReceiver.h"

/// Constructor 
Encoder::Encoder(Pipeline &pipeline, const char *encoder) : 
    pipeline_(pipeline), encoder_(pipeline_.makeElement(encoder, NULL))
{}

Encoder::Encoder(Pipeline &pipeline) : 
    pipeline_(pipeline), encoder_(0) // for encoder-less raw
{}


/// Destructor 
Encoder::~Encoder()
{
    pipeline_.remove(&encoder_);
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
    if (pipeline_.isPlaying())
    {
        pipeline_.makeReady();
        g_object_set(G_OBJECT(encoder_), "bitrate", bitrate, NULL);
        pipeline_.start();
    }
    else
    {
        LOG_DEBUG("SETTING BITRATE TO " << bitrate);
        g_object_set(G_OBJECT(encoder_), "bitrate", bitrate, NULL);
    }
}

/// Constructor 
Decoder::Decoder(Pipeline &pipeline, const char *decoder) : 
    pipeline_(pipeline), decoder_(pipeline_.makeElement(decoder, NULL))
{}


/// Constructor 
Decoder::Decoder(Pipeline &pipeline) : 
    pipeline_(pipeline), decoder_(0) // for decoderless raw
{}


/// Destructor 
Decoder::~Decoder()
{
    pipeline_.remove(&decoder_);
}


VideoEncoder::VideoEncoder(Pipeline &pipeline, const char *encoder, bool supportsInterlaced) :
    Encoder(pipeline, encoder),
    colorspace_(pipeline_.makeElement("ffmpegcolorspace", NULL)), 
    supportsInterlaced_(supportsInterlaced)  // most codecs don't have this property
{
    tassert(encoder_);
    if (supportsInterlaced_)  // not all encoders have this property
        g_object_set(encoder_, "interlaced", TRUE, NULL); // true if we are going to encode interlaced material

    gstlinkable::link(colorspace_, encoder_);
}

/// Destructor 
VideoEncoder::~VideoEncoder()
{
    pipeline_.remove(&colorspace_);
}

VideoDecoder::VideoDecoder(Pipeline &pipeline, const char *decoder, bool doDeinterlace) : 
    Decoder(pipeline, decoder),
    doDeinterlace_(doDeinterlace), 
    colorspace_(0), 
    deinterlace_(0)
{}


/// Destructor 
VideoDecoder::~VideoDecoder()
{
    pipeline_.remove(&colorspace_);
    pipeline_.remove(&deinterlace_);
}


/// Sets up either decoder->colorspace->deinterlace
void VideoDecoder::addDeinterlace()
{
    // FIXME: should maybe be settable
    enum {ALL = 0, TOP, BOTTOM}; // deinterlace produces all fields, or top, bottom

    tassert(decoder_ != 0);
    if (doDeinterlace_)
    {
        colorspace_ = pipeline_.makeElement("ffmpegcolorspace", NULL); 
        LOG_DEBUG("DO THE DEINTERLACE");
        deinterlace_ = pipeline_.makeElement("deinterlace", NULL);
        g_object_set(deinterlace_, "fields", TOP, NULL);
        gstlinkable::link(decoder_, colorspace_);
        gstlinkable::link(colorspace_, deinterlace_);
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

H264Encoder::H264Encoder(Pipeline &pipeline, int bitrate) : 
    VideoEncoder(pipeline, "x264enc", true),
    bitrate_(bitrate) 
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
    return new H264Pay(pipeline_);
}


H264Decoder::H264Decoder(Pipeline &pipeline, bool doDeinterlace) : 
    VideoDecoder(pipeline, "ffdec_h264", doDeinterlace)
{
    addDeinterlace();
}


/// Creates an h.264 RtpDepay 
RtpPay* H264Decoder::createDepayloader() const
{
    return new H264Depay(pipeline_);
}


/// Increase jitterbuffer size
void H264Decoder::adjustJitterBuffer() 
{
    RtpReceiver::setLatency(LONGER_JITTER_BUFFER_MS);
}



/// Constructor 
H263Encoder::H263Encoder(Pipeline &pipeline, int bitrate) : 
    VideoEncoder(pipeline, "ffenc_h263p", false),
    bitrate_(bitrate)
{
    setBitrate(bitrate_);
}


/// Destructor 
H263Encoder::~H263Encoder()
{}


/// Creates an h.263 rtp payloader 
Pay* H263Encoder::createPayloader() const
{
    return new H263Pay(pipeline_);
}


H263Decoder::H263Decoder(Pipeline &pipeline, bool doDeinterlace) : 
    VideoDecoder(pipeline, "ffdec_h263", doDeinterlace)
{
    addDeinterlace();
}


/// Creates an h.263 RtpDepay 
RtpPay* H263Decoder::createDepayloader() const
{
    return new H263Depay(pipeline_);
}


/// Constructor 
Mpeg4Encoder::Mpeg4Encoder(Pipeline &pipeline, int bitrate) : 
    VideoEncoder(pipeline, "ffenc_mpeg4", false), // FIXME: interlaced may cause stuttering
    bitrate_(bitrate)
{
    setBitrate(bitrate_);
}


/// Destructor 
Mpeg4Encoder::~Mpeg4Encoder()
{}


/// Creates an h.264 rtp payloader 
Pay* Mpeg4Encoder::createPayloader() const
{
    return new Mpeg4Pay(pipeline_);
}


Mpeg4Decoder::Mpeg4Decoder(Pipeline &pipeline, bool doDeinterlace) : 
    VideoDecoder(pipeline, "ffdec_mpeg4", doDeinterlace)
{
    addDeinterlace();
}


/// Creates an mpeg4 RtpDepay 
RtpPay* Mpeg4Decoder::createDepayloader() const
{
    return new Mpeg4Depay(pipeline_);
}


/// Constructor 
TheoraEncoder::TheoraEncoder(Pipeline &pipeline, int bitrate, int quality) : 
    VideoEncoder(pipeline, "theoraenc", false),
    bitrate_(bitrate), 
    quality_(quality) 
{
    setSpeedLevel(MAX_SPEED_LEVEL);
    //g_object_set(encoder_, "keyframe-force", 1, NULL);
    if (bitrate_ != 0)
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
    return new TheoraPay(pipeline_);
}


TheoraDecoder::TheoraDecoder(Pipeline &pipeline, bool doDeinterlace) : 
    VideoDecoder(pipeline, "theoradec", doDeinterlace)
{
    addDeinterlace();
}

RtpPay* TheoraDecoder::createDepayloader() const
{
    return new TheoraDepay(pipeline_);
}

/// Constructor 
VorbisEncoder::VorbisEncoder(Pipeline &pipeline) :
    Encoder(pipeline, "vorbisenc")
{
}


/// Destructor 
VorbisEncoder::~VorbisEncoder() 
{}

/// Creates an RtpVorbisPay 
Pay* VorbisEncoder::createPayloader() const
{
    return new VorbisPay(pipeline_);
}


unsigned long long VorbisDecoder::minimumBufferTime()
{
    return MIN_BUFFER_USEC;
}


VorbisDecoder::VorbisDecoder(Pipeline &pipeline) :
    Decoder(pipeline, "vorbisdec")
{}


/// Creates an RtpVorbisDepay 
RtpPay* VorbisDecoder::createDepayloader() const
{
    return new VorbisDepay(pipeline_);
}

/// Constructor
RawEncoder::RawEncoder(Pipeline &pipeline) : 
    Encoder(pipeline),
    aconv_(pipeline_.makeElement("audioconvert", NULL))
{}

/// Destructor
RawEncoder::~RawEncoder()
{
    pipeline_.remove(&aconv_);
}

/// Creates an RtpL16Pay 
Pay* RawEncoder::createPayloader() const
{
    return new L16Pay(pipeline_);
}

/// Constructor
RawDecoder::RawDecoder(Pipeline &pipeline) :
    Decoder(pipeline),
    aconv_(pipeline_.makeElement("audioconvert", NULL))
{}


/// Destructor
RawDecoder::~RawDecoder()
{
    pipeline_.remove(&aconv_);
}

/// Creates an RtpL16Depay 
RtpPay* RawDecoder::createDepayloader() const
{
    return new L16Depay(pipeline_);
}

/// Constructor
LameEncoder::LameEncoder(Pipeline &pipeline) : 
    Encoder(pipeline, "lamemp3enc"),
    aconv_(pipeline_.makeElement("audioconvert", NULL)),
    mp3parse_(pipeline_.makeElement("mp3parse", NULL))
{
    gstlinkable::link(aconv_, encoder_);
    gstlinkable::link(encoder_, mp3parse_);
}


/// Destructor
LameEncoder::~LameEncoder()
{
    pipeline_.remove(&mp3parse_);
    pipeline_.remove(&aconv_);
}

/// Constructor
MadDecoder::MadDecoder(Pipeline &pipeline) :
    Decoder(pipeline, "mad"),
    aconv_(pipeline_.makeElement("audioconvert", NULL))
{
    gstlinkable::link(decoder_, aconv_);
}

MadDecoder::~MadDecoder()
{
    pipeline_.remove(&aconv_);
}


/** 
 * Creates an RtpMpaPay */
Pay* LameEncoder::createPayloader() const
{
    return new MpaPay(pipeline_);
}

/// Creates an RtpMpaDepay 
RtpPay* MadDecoder::createDepayloader() const
{
    return new MpaDepay(pipeline_);
}

