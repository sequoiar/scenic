
// audioConfig.cpp
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
#include <iostream>
#include <fstream>
#include "audioConfig.h"
#include "audioSource.h"
#include "audioSink.h"
#include "logWriter.h"
//#include "audioDelaySource.h"

// strips .delay from source name
const char *AudioConfig::source() const
{
    // find last period
    unsigned int pos = source_.rfind(".");

    if (pos < source_.size())
        return source_.substr(0, pos).c_str();
    else
        return source_.c_str();
}


AudioSource* AudioConfig::createSource() const
{
#if 0
    if (source_ == "audiotestsrc.delay")
        return new AudioDelaySource<AudioTestSource>(*this);
    else if (source_ == "filesrc.delay")
        return new AudioDelaySource<AudioFileSource>(*this);
    else if (source_ == "alsasrc.delay")
        return new AudioDelaySource<AudioAlsaSource>(*this);
    else if (source_ == "jackaudiosrc.delay")
        return new AudioDelaySource<AudioJackSource>(*this);
#endif
    if (source_ == "audiotestsrc")
        return new AudioTestSource(*this);
    else if (source_ == "filesrc")
        return new AudioFileSource(*this);
    else if (source_ == "alsasrc")
        return new AudioAlsaSource(*this);
    else if (source_ == "jackaudiosrc")
        return new AudioJackSource(*this);
    else if (source_ == "dv1394src")
        return new AudioDvSource(*this);
    else 
        THROW_ERROR(source_ << " is an invalid source");
    return 0;
}

// FIXME: should be paramaterized by sink, not src
AudioSink* AudioConfig::createSink() const
{
    if (source_ == "alsasrc")
        return new AudioAlsaSink();
    else if ((!source_.empty()) && 
            (source_ != "jackaudiosrc") && 
            (source_ != "dv1394src") && 
            (source_ != "audiotestsrc") && 
            (source_ != "filesrc"))
        LOG_WARNING(source_ << " is an invalid sink, using default jackaudiosink");

    return new AudioJackSink();
}


bool AudioConfig::sanityCheck() const   // FIXME: this should become more or less redundant
{
    if(numChannels_ < 1 || numChannels_ > 8)
        THROW_CRITICAL("Invalid number of channels");

    return true;
}


const char* AudioConfig::location() const
{
    if (location_.empty())
        THROW_ERROR("No location specified");
        
    return location_.c_str();
}

bool AudioConfig::fileExists() const
{
    if (location_.empty())
        THROW_ERROR("No file location given");

    std::fstream in;
    in.open(location(), std::fstream::in);

    if (in.fail())
        THROW_ERROR("File does not exist and/or is not readable.");

    in.close();
    return true;
}


AudioSink* AudioReceiverConfig::createSink() const
{
    if (sink_ == "jackaudiosink")
        return new AudioJackSink();
    else if (sink_ == "alsasink")
        return new AudioAlsaSink();
    else if (sink_.empty()) 
    {
        LOG_WARNING("No sink specified, using default jackaudiosink");
        return new AudioJackSink();
    }
    else
    {
        LOG_WARNING(sink_ << " is an invalid sink, using default jackaudiosink");
        return new AudioJackSink();
    }
}


