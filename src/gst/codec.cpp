/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <algorithm> // for std::find
#include <cmath> // for std::fabs
#include <sstream> // for ostringstream

#include <gst/gst.h>
#include <gst/audio/multichannel.h>
#include "util/log_writer.h"
#include "gutil/gutil.h"

#include "gst_linkable.h"

#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>

#include "codec.h"
#include "jack_util.h"
#include "rtp_pay.h"
#include "pipeline.h"

#include "rtp_receiver.h"

/// Constructor
Encoder::Encoder(const Pipeline &pipeline, const char *encoder) :
    pipeline_(pipeline), encoder_(pipeline_.makeElement(encoder, NULL))
{}

Encoder::Encoder(const Pipeline &pipeline) :
    pipeline_(pipeline), encoder_(0) // for encoder-less raw
{}


/// Destructor
Encoder::~Encoder()
{
}

/// Returns bitrate property for this encoder
int Encoder::getBitrate() const
{
    assert(encoder_);
    int bitrate;
    g_object_get(G_OBJECT(encoder_), "bitrate", &bitrate, NULL);
    return bitrate;
}

/// Sets bitrate property for this encoder
void Encoder::setBitrate(int bitrate)
{
    assert(encoder_);
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
Decoder::Decoder(const Pipeline &pipeline, const char *decoder) :
    pipeline_(pipeline), decoder_(pipeline_.makeElement(decoder, NULL))
{}


/// Constructor
Decoder::Decoder(const Pipeline &pipeline) :
    pipeline_(pipeline), decoder_(0) // for decoderless raw
{}


/// Destructor
Decoder::~Decoder()
{
}

VideoEncoder::VideoEncoder(const Pipeline &pipeline, const char *encoder, bool supportsInterlaced) :
    Encoder(pipeline, encoder),
    colorspace_(pipeline_.makeElement("ffmpegcolorspace", NULL)),
    supportsInterlaced_(supportsInterlaced)  // most codecs don't have this property
{
    assert(encoder_);
    if (supportsInterlaced_)  // not all encoders have this property
        g_object_set(encoder_, "interlaced", TRUE, NULL); // true if we are going to encode interlaced material

    gstlinkable::link(colorspace_, encoder_);
}

VideoDecoder::VideoDecoder(const Pipeline &pipeline, const char *decoder, bool doDeinterlace) :
    Decoder(pipeline, decoder),
    doDeinterlace_(doDeinterlace),
    colorspace_(0),
    deinterlace_(0)
{}


/// Sets up either decoder->colorspace->deinterlace
void VideoDecoder::addDeinterlace()
{
    // FIXME: should maybe be settable
    enum {ALL = 0, TOP, BOTTOM}; // deinterlace produces all fields, or top, bottom

    assert(decoder_);
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

/// Constructor
// POSIX specific hardware thread info
// http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
// int numThreads = sysconf(_SC_NPROCESSORS_ONLN);

H264Encoder::H264Encoder(const Pipeline &pipeline, int bitrate) :
    VideoEncoder(pipeline, "x264enc", true),
    bitrate_(bitrate)
{
    g_object_set(encoder_, "threads", 0 /* automatic */, NULL);
    // See gst-plugins-good/tests/examples/rtp/*h264*.sh
    // if you use non-byte stream mode, the encoder willl need to add
    // three bytes to start and end, which the payerloader will promptly
    // remove (as the buffer size is givn on the buffer object). the
    // default NALU stream is mostly useful when storing this on disk
    // i.e. (x264enc ! filesink)
    // vbv-bufsize / vbv-maxrate = the number of seconds the client must buffer before playback
    g_object_set(encoder_, "byte-stream", TRUE, NULL);
    g_object_set(encoder_, "vbv-buf-capacity", 300, "intra-refresh", TRUE, NULL);
    // These are x264enc types (i.e. enums) so we can't use g_object_set
    // directly
    gst_util_set_object_arg (G_OBJECT(encoder_), "tune", "zerolatency");
    gst_util_set_object_arg (G_OBJECT(encoder_), "pass", "qual");

    // subme: subpixel motion estimation 1=fast, 6=best

    setBitrate(bitrate_);
}


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


H264Decoder::H264Decoder(const Pipeline &pipeline, bool doDeinterlace) :
    VideoDecoder(pipeline, "ffdec_h264", doDeinterlace)
{
    addDeinterlace();
}


/// Creates an h.264 RtpDepay
RtpPay* H264Decoder::createDepayloader() const
{
    return new H264Depay(pipeline_);
}


/// Constructor
H263Encoder::H263Encoder(const Pipeline &pipeline, int bitrate) :
    VideoEncoder(pipeline, "ffenc_h263p", false),
    bitrate_(bitrate)
{
    setBitrate(bitrate_);
}


/// Creates an h.263 rtp payloader
Pay* H263Encoder::createPayloader() const
{
    return new H263Pay(pipeline_);
}


H263Decoder::H263Decoder(const Pipeline &pipeline, bool doDeinterlace) :
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
Mpeg4Encoder::Mpeg4Encoder(const Pipeline &pipeline, int bitrate) :
    VideoEncoder(pipeline, "ffenc_mpeg4", false), // FIXME: interlaced may cause stuttering
    bitrate_(bitrate)
{
    setBitrate(bitrate_);
}


/// Creates an h.264 rtp payloader
Pay* Mpeg4Encoder::createPayloader() const
{
    return new Mpeg4Pay(pipeline_);
}


Mpeg4Decoder::Mpeg4Decoder(const Pipeline &pipeline, bool doDeinterlace) :
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
TheoraEncoder::TheoraEncoder(const Pipeline &pipeline, int bitrate, int quality) :
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


/// Overridden to convert from bit/s to kbit/s
void TheoraEncoder::setBitrate(int newBitrate)
{
    LOG_DEBUG("Bitrate " << newBitrate);
    Encoder::setBitrateInKbs(newBitrate);
}


// theora specific
void TheoraEncoder::setQuality(int quality)
{
    if (quality >= MIN_QUALITY and quality <= MAX_QUALITY)
    {
        LOG_DEBUG("Quality " << quality);
        g_object_set(encoder_, "quality", quality, NULL);
    }
}


// theora specific
void TheoraEncoder::setSpeedLevel(int speedLevel)
{
    assert(encoder_);
    if (speedLevel < MIN_SPEED_LEVEL or speedLevel > MAX_SPEED_LEVEL)
        THROW_ERROR("Speed-level must be in range [" << MIN_SPEED_LEVEL << "-" << MAX_SPEED_LEVEL << "]");
    g_object_set(encoder_, "speed-level", speedLevel, NULL);
}


Pay* TheoraEncoder::createPayloader() const
{
    return new TheoraPay(pipeline_);
}


TheoraDecoder::TheoraDecoder(const Pipeline &pipeline, bool doDeinterlace) :
    VideoDecoder(pipeline, "theoradec", doDeinterlace)
{
    addDeinterlace();
}

RtpPay* TheoraDecoder::createDepayloader() const
{
    return new TheoraDepay(pipeline_);
}


/// Constructor
CeltEncoder::CeltEncoder(const Pipeline &pipeline, int /*bitrate*/)
    : Encoder(pipeline, "celtenc"),
    audioconvert_(pipeline.makeElement("audioconvert", 0))
{
    gstlinkable::link(audioconvert_, encoder_);
    /// FIXME: check and set bitrate
}

/// Creates an RtpCeltPay
Pay* CeltEncoder::createPayloader() const
{
    return new CeltPay(pipeline_);
}

CeltDecoder::CeltDecoder(const Pipeline &pipeline) :
    Decoder(pipeline, "celtdec"),
    audioconvert_(pipeline_.makeElement("audioconvert", 0))
{
    gstlinkable::link(decoder_, audioconvert_);
}

/// Creates an RtpCeltDepay
RtpPay* CeltDecoder::createDepayloader() const
{
    return new CeltDepay(pipeline_);
}

/// Constructor
VorbisEncoder::VorbisEncoder(const Pipeline &pipeline, int bitrate, double quality)
    :
        Encoder(pipeline, "vorbisenc"),
        queue_(pipeline.makeElement("queue", 0))
{
    gstlinkable::link(queue_, encoder_);
    static const double MIN_QUALITY = -0.1;  // from the vorbis plugin
    static const double MAX_QUALITY = 1.0;  // from the vorbis plugin
    static const int MIN_BITRATE = 0;
    static const int BITS_PER_KB = 1024;
    if (quality >= MIN_QUALITY and quality <= MAX_QUALITY)
        g_object_set(encoder_, "quality", quality, NULL);
    else if (bitrate > MIN_BITRATE)
        g_object_set(encoder_, "bitrate", bitrate * BITS_PER_KB, NULL);
}

/// Creates an RtpVorbisPay
Pay* VorbisEncoder::createPayloader() const
{
    return new VorbisPay(pipeline_);
}

VorbisDecoder::VorbisDecoder(const Pipeline &pipeline) :
    Decoder(pipeline, "vorbisdec")
{}

/// Creates an RtpVorbisDepay
RtpPay* VorbisDecoder::createDepayloader() const
{
    return new VorbisDepay(pipeline_);
}

/// Constructor
RawEncoder::RawEncoder(const Pipeline &pipeline) :
    Encoder(pipeline),
    aconv_(pipeline_.makeElement("audioconvert", NULL))
{}

/// Creates an RtpL16Pay
Pay* RawEncoder::createPayloader() const
{
    return new L16Pay(pipeline_);
}

/// Constructor
RawDecoder::RawDecoder(const Pipeline &pipeline, int numChannels) :
    Decoder(pipeline),
    aconv_(pipeline_.makeElement("audioconvert", NULL)),
    // FIXME: SUPER GROSS HACK!!!! We should not need a capsfilter here,
    // something is broken in gst
    capsfilter_(pipeline_.makeElement("capsfilter", NULL))
{
    gutil::initAudioCapsFilter(capsfilter_, numChannels);
    gstlinkable::link(aconv_, capsfilter_);
}

/// Creates an RtpL16Depay
RtpPay* RawDecoder::createDepayloader() const
{
    return new L16Depay(pipeline_);
}

/// Constructor
LameEncoder::LameEncoder(const Pipeline &pipeline, int bitrate, double quality)
    :
        Encoder(pipeline, "lamemp3enc"),
        aconv_(pipeline_.makeElement("audioconvert", NULL)),
        mp3parse_(pipeline_.makeElement("mp3parse", NULL))
{
    static const double SCALE = 10.0;    // from the lame plugin
    static const double MIN_QUALITY = 0.01;  // from the lame plugin
    static const double MAX_QUALITY = 1.0;  // from the lame plugin

    using namespace boost::assign;
    std::vector<int> allowedBitrates;
    allowedBitrates += 7, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320;

    if (quality >= MIN_QUALITY and quality <= MAX_QUALITY)
    {
        g_object_set(encoder_, "target", "quality", NULL);
        g_object_set(encoder_, "quality", std::fabs(SCALE - (quality * SCALE)), NULL);
    }
    else if (std::find(allowedBitrates.begin(), allowedBitrates.end(), bitrate) != allowedBitrates.end())
    {
        g_object_set(encoder_, "target", "quality", NULL);
        g_object_set(encoder_, "bitrate", "bitrate", NULL);
    }
    else if (bitrate > 0) // 0 means unused, so ignore it
    {
        std::ostringstream str;
        std::copy(allowedBitrates.begin(), allowedBitrates.end(), std::ostream_iterator<int>(str, " "));
        LOG_WARNING("Ignoring invalid bitrate value of " <<
                bitrate << ", allowed bitrates are:\n " << str.str());
    }

    gstlinkable::link(aconv_, encoder_);
    gstlinkable::link(encoder_, mp3parse_);
}

/// Constructor
MadDecoder::MadDecoder(const Pipeline &pipeline) :
    Decoder(pipeline, "mad"),
    aconv_(pipeline_.makeElement("audioconvert", NULL))
{
    gstlinkable::link(decoder_, aconv_);
}

/// Creates an RtpMpaPay
Pay* LameEncoder::createPayloader() const
{
    return new MpaPay(pipeline_);
}

/// Creates an RtpMpaDepay
RtpPay* MadDecoder::createDepayloader() const
{
    return new MpaDepay(pipeline_);
}

// anonymous namespace to hide this function
namespace {
int maxRawChannels()
{
    if (not Jack::is_running())
    {
        LOG_WARNING("Jack is not running");
        return INT_MAX;
    }

    GstElement *fakePipeline = gst_parse_launch("jackaudiosrc connect=0 name=fakejackaudiosrc ! fakesink silent=true", 0);
    gst_element_set_state(fakePipeline, GST_STATE_PAUSED);
    GstElement *element = gst_bin_get_by_name(GST_BIN(fakePipeline), "fakejackaudiosrc");
    GstPad *srcPad = gst_element_get_static_pad(element, "src");
    GstCaps *srcCaps;
    while ((srcCaps = gst_pad_get_negotiated_caps(srcPad)) == NULL)
        LOG_DEBUG("not ready\n");
    GstStructure *structure = gst_caps_get_structure(srcCaps, 0);
    gint result;
    if (not gst_structure_has_field(structure, "channel-positions"))
    {
        result = 8;
        LOG_DEBUG("jackaudiosrc does not set channel-positions, so the maximum number of channels we can send is " << result);
    }
    else
    {
        result = INT_MAX;
        LOG_DEBUG("jackaudiosrc sets channel-positions, so the maximum number of channels we can send is " << result);
    }

    gst_element_set_state(fakePipeline, GST_STATE_NULL);
    gst_caps_unref(srcCaps);
    gst_object_unref(srcPad);
    gst_object_unref(GST_OBJECT(fakePipeline));
    return result;
}
}

/// FIXME: this is audio specific but could go anywhere
int Encoder::maxChannels(const std::string &codec)
{
    int result = 0;
    if (codec == "mp3" or codec == "celt")
        result = 2;
    else if (codec == "raw")
        result = maxRawChannels();
    else if (codec == "vorbis")
        result = 256;
    else
        LOG_ERROR("Invalid codec " << codec);
    return result;
}

