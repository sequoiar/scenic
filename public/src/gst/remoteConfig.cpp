
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


Encoder * SenderConfig::createEncoder() const
{
    if (codec_.empty())
        THROW_ERROR("Can't make encoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisEncoder();
    if (codec_ == "h264")
        return new H264Encoder();
    
    THROW_ERROR(codec_ << " is an invalid codec!");
    return 0;
}


Decoder * ReceiverConfig::createDecoder() const
{
    if (codec_.empty())
        THROW_ERROR("Can't make decoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisDecoder();
    if (codec_ == "h264")
        return new H264Decoder();
    
    THROW_ERROR(codec_ << " is an invalid codec!");
    return 0;
}

#if 0
bool RemoteConfig::sanityCheck() const   // FIXME: this should become more or less redundant
{
    bool validCodec = (codec_ == "vorbis") || (codec_ == "h264"); 

    if(codec_.empty())
        THROW_ERROR("No Codec specified.");
    if(!validCodec)
        THROW_ERROR("Bad codec:" << codec_);
    if (port_ == -1)
        THROW_ERROR("No Port, one needed");  
    
    return validCodec;
}
#endif
