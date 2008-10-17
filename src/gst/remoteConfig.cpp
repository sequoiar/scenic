
// remoteConfig.cpp
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

/** \file
 *      Class for audio parameter objects.
 *
 */
#include "remoteConfig.h"
#include "codec.h"
#include "logWriter.h"
#include <string>
#include <algorithm>

const int RemoteConfig::PORT_MIN = 1024;
const int RemoteConfig::PORT_MAX = 65000;

const std::string RemoteConfig::VALID_CODECS[NUM_CODECS] = {"h264", "raw", "vorbis", "mp3"};
        
RemoteConfig::RemoteConfig(const std::string &codec__, const std::string &remoteHost__,
        int port__) : codec_(codec__), remoteHost_(remoteHost__), port_(port__)
{
    if(codec_.empty())
        THROW_ERROR("No Codec specified.");
    bool validCodec = std::find(VALID_CODECS, VALID_CODECS + NUM_CODECS*sizeof(std::string), codec_);
    //bool validCodec = iter != VALID_CODECS.end();

    if(!validCodec)
        THROW_ERROR("Bad codec:" << codec_);
    if (port_ < PORT_MIN || port_ > PORT_MAX)
        THROW_ERROR("Invalid port " << port_ << ", must be in range [" 
                << PORT_MIN << "," << PORT_MAX << "]");  

}


Encoder * SenderConfig::createEncoder() const
{
    if (codec_.empty())
        THROW_ERROR("Can't make encoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisEncoder();
    else if (codec_ == "h264")
        return new H264Encoder();
    else if (codec_ == "raw")
        return new RawEncoder();
    else if (codec_ == "mp3")
        return new LameEncoder();

    THROW_ERROR(codec_ << " is an invalid codec!");
    return 0;
}


Decoder * ReceiverConfig::createDecoder() const
{
    if (codec_.empty())
        THROW_ERROR("Can't make decoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisDecoder();
    else if (codec_ == "h264")
        return new H264Decoder();
    else if (codec_ == "raw")
        return new RawDecoder();
    else if (codec_ == "mp3")
        return new MadDecoder();

    THROW_ERROR(codec_ << " is an invalid codec!");
    return 0;
}


