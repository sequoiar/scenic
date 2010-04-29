/* remoteConfig.cpp
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

#include "util.h"

#include <algorithm>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <gst/gst.h>
#include "pipeline.h"
#include "remoteConfig.h"
#include "tcp/CapsServer.h"
#include "tcp/CapsClient.h"
#include "codec.h"

const int RemoteConfig::PORT_MIN = 1024;
const int RemoteConfig::PORT_MAX = 65000;

std::set<int> RemoteConfig::usedPorts_;
        
RemoteConfig::RemoteConfig(const std::string &codec__,
        const std::string &remoteHost__,
        int port__) :
    codec_(codec__), 
    remoteHost_(remoteHost__), 
    port_(port__)
{}


// Can't be called from destructor, must be called by this object's owner/client, 
// as sometime this object is copied and we don't want the ports destroyed prematurely
void RemoteConfig::cleanupPorts() const
{
    usedPorts_.erase(capsPort());
    usedPorts_.erase(rtcpSecondPort());
    usedPorts_.erase(rtcpFirstPort());
    usedPorts_.erase(port());
}


/// Make sure there are no port clashes (at least as far as this process can tell)
void RemoteConfig::checkPorts() const
{
    if (port_ < PORT_MIN || port_ > PORT_MAX)
        THROW_ERROR("Invalid port " << port_ << ", must be in range [" 
                << PORT_MIN << "," << PORT_MAX << "]");  

    if (usedPorts_.find(port()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", already in use");

    if (usedPorts_.find(rtcpFirstPort()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", its rtcp port " << rtcpFirstPort() << " is already in use");

    if (usedPorts_.find(rtcpSecondPort()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", its rtcp port " << rtcpSecondPort() << " is already in use");

    if (usedPorts_.find(capsPort()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", its caps port " << capsPort() << " is already in use");

    // add our ports now that we know they're available
    usedPorts_.insert(port());
    usedPorts_.insert(rtcpFirstPort());
    usedPorts_.insert(rtcpSecondPort());
    usedPorts_.insert(capsPort());
}
        

SenderConfig::SenderConfig(Pipeline &pipeline,
        const std::string &codec__,
        const std::string &remoteHost__,
        int port__) :
    RemoteConfig(codec__, remoteHost__, port__),
    BusMsgHandler(&pipeline),
    message_(""), 
    capsOutOfBand_(false),    // this will be determined later
    capsServer_()
{}


VideoEncoder * SenderConfig::createVideoEncoder(const Pipeline &pipeline, int bitrate, int quality) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make encoder without codec being specified.");

    if (codec_ == "h264")
        return new H264Encoder(pipeline, bitrate);
    else if (codec_ == "h263")
        return new H263Encoder(pipeline, bitrate);       // set caps from here?
    else if (codec_ == "mpeg4")
        return new Mpeg4Encoder(pipeline, bitrate);
    else if (codec_ == "theora")
        return new TheoraEncoder(pipeline, bitrate, quality);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
    LOG_DEBUG("Video encoder " << codec_ << " built"); 
}


Encoder * SenderConfig::createAudioEncoder(const Pipeline &pipeline, int bitrate, double quality) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make encoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisEncoder(pipeline, bitrate, quality);
    else if (codec_ == "raw")
        return new RawEncoder(pipeline);
    else if (codec_ == "mp3")
        return new LameEncoder(pipeline, bitrate, quality);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
    LOG_DEBUG("Audio encoder " << codec_ << " built"); 
}


void SenderConfig::sendMessage() 
{
    capsServer_.reset(new CapsServer(capsPort(), message_));
}


/** 
 * The new caps message is posted on the bus by the src pad of our udpsink, 
 * received by this audiosender, and sent to our other host if needed. */
bool SenderConfig::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    if (s != NULL and gst_structure_has_name(s, "caps-changed"))
    {   
        // this is our msg
        const gchar *newCapsStr = gst_structure_get_string(s, "caps");
        tassert(newCapsStr);
        std::string str(newCapsStr);

        GstStructure *structure = gst_caps_get_structure(gst_caps_from_string(str.c_str()), 0);
        const GValue *encodingStr = gst_structure_get_value(structure, "encoding-name");
        std::string encodingName(g_value_get_string(encodingStr));

        if (!capsMatchCodec(encodingName, codec()))
            return false;   // not our caps, ignore it
        else if (capsOutOfBand_) 
        { 
            LOG_DEBUG("Creating caps server for codec " << codec());
            message_ = std::string(newCapsStr);
            sendMessage();
            return true;
        }
        else
            return true;       // was our caps, but we don't need to send caps for it
    }

    return false;           // this wasn't our msg, someone else should handle it
}

static const std::vector<std::string> AUDIO_CODECS = 
boost::assign::list_of<std::string>("raw")("mp3")("vorbis");
static const std::vector<std::string> VIDEO_CODECS = 
boost::assign::list_of<std::string>("mpeg4")("h264")("h263")("theora");

/// FIXME: this method and the one below it need a list of codecs, should only have one
std::string RemoteConfig::codecMediaType() const
{
    using std::string;
    string result;

    if (std::find(AUDIO_CODECS.begin(), AUDIO_CODECS.end(), codec_) != AUDIO_CODECS.end())
        result = "audio";
    else if (std::find(VIDEO_CODECS.begin(), VIDEO_CODECS.end(), codec_) != VIDEO_CODECS.end())
        result = "video";
    else
        LOG_ERROR("Unknown codec " << codec_);
    return result;
}

std::string RemoteConfig::identifier() const
{
    return codecMediaType() + "_" + codec_;  
}


bool ReceiverConfig::isSupportedCodec(const std::string &codec)
{
    bool result = std::find(AUDIO_CODECS.begin(), AUDIO_CODECS.end(), codec) != AUDIO_CODECS.end()
        or std::find(VIDEO_CODECS.begin(), VIDEO_CODECS.end(), codec) != VIDEO_CODECS.end();
    return result;
}


ReceiverConfig::ReceiverConfig(const std::string &codec__,
        const std::string &remoteHost__,
        int port__,
        const std::string &multicastInterface__,
        bool negotiateCaps,
        bool enableControls,
        const std::string &caps__) :
    RemoteConfig(codec__, remoteHost__, port__), 
    multicastInterface_(multicastInterface__),
    caps_(caps__), 
    capsOutOfBand_(negotiateCaps or caps_ == ""),
    jitterbufferControlEnabled_(enableControls)
{
    if (capsOutOfBand_) // couldn't find caps, need them from other host or we've explicitly been told to send caps
    {
        if (isSupportedCodec(codec_))   // this would fail later but we want to make sure we don't wait with a bogus codec
        { 
            LOG_INFO("Waiting for " << codec_ << " caps from other host");
            receiveCaps();  // wait for new caps from sender
        }
        else
            THROW_ERROR("Codec " << codec_ << " is not supported");
    }
}

VideoDecoder * ReceiverConfig::createVideoDecoder(const Pipeline &pipeline, bool doDeinterlace) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make decoder without codec being specified.");

    if (codec_ == "h264")
        return new H264Decoder(pipeline, doDeinterlace);
    else if (codec_ == "h263")
        return new H263Decoder(pipeline, doDeinterlace);
    else if (codec_ == "mpeg4")
        return new Mpeg4Decoder(pipeline, doDeinterlace);
    else if (codec_ == "theora")
        return new TheoraDecoder(pipeline, doDeinterlace);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
}


Decoder * ReceiverConfig::createAudioDecoder(const Pipeline &pipeline, int numChannels) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make decoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisDecoder(pipeline);
    else if (codec_ == "raw")
        return new RawDecoder(pipeline, numChannels);
    else if (codec_ == "mp3")
        return new MadDecoder(pipeline);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
}


/// compares internal codec names and RTP-header codec names
bool RemoteConfig::capsMatchCodec(const std::string &encodingName, const std::string &codec)
{
    return (encodingName == "VORBIS" and codec == "vorbis")
        or (encodingName == "L16" and codec == "raw")
        or (encodingName == "MPA" and codec == "mp3")
        or (encodingName == "MP4V-ES" and codec == "mpeg4")
        or (encodingName == "H264" and codec == "h264")
        or (encodingName == "H263-1998" and codec == "h263")
        or (encodingName == "THEORA" and codec == "theora");
}

/// This function makes sure that the caps set on this receiver by a sender, match the codec
/// that it expects. If it fails, it is probably due to a mismatch of codecs between sender and
/// receiver, or a change in the caps specification for a given codec from gstreamer.
//  TODO: maybe the whole receiver should be created based on this info, at least up to and including
//  the decoder?
bool ReceiverConfig::capsMatchCodec() const
{
    GstStructure *structure = gst_caps_get_structure(gst_caps_from_string(caps_.c_str()), 0);
    const GValue *str = gst_structure_get_value(structure, "encoding-name");
    std::string encodingName(g_value_get_string(str));

    return RemoteConfig::capsMatchCodec(encodingName, codec_);
}


void ReceiverConfig::receiveCaps()
{
    // this blocks
    LOG_DEBUG("Creating new caps client to get caps from " << remoteHost_);
    CapsClient capsClient(remoteHost_, boost::lexical_cast<std::string>(capsPort()));
    caps_ = capsClient.getCaps();
    LOG_DEBUG("Received caps " << caps_);
}

