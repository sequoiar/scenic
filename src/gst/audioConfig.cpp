
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

/** \file audioConfig.cpp
 *      Immutable class for audio parameter objects.
 */

#include <iostream>
#include <fstream>
#include "audioConfig.h"
#include "audioSource.h"
#include "audioSink.h"
#include "logWriter.h"

const char *AudioSourceConfig::source() const
{
    return source_.c_str();
}

AudioSource* AudioSourceConfig::createSource() const
{
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
    else if (source_ == "pulsesrc")
        return new AudioPulseSource(*this);
    else 
        THROW_ERROR(source_ << " is an invalid source");
    return 0;
}

// FIXME: should be paramaterized by sink, not src
AudioSink* AudioSourceConfig::createSink() const
{
    if (source_ == "alsasrc")
        return new AudioAlsaSink();
    else if (source_ == "pulsesrc")
        return new AudioPulseSink();
    else if ((!source_.empty()) && 
            (source_ != "jackaudiosrc") && 
            (source_ != "dv1394src") && 
            (source_ != "audiotestsrc") && 
            (source_ != "filesrc"))
        LOG_WARNING(source_ << " is an invalid sink, using default jackaudiosink");

    return new AudioJackSink();
}


const char* AudioSourceConfig::location() const
{
    if (location_.empty())
        THROW_ERROR("No location specified");

    return location_.c_str();
}


bool AudioSourceConfig::fileExists() const
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


AudioSink* AudioSinkConfig::createSink() const
{
    if (sink_ == "jackaudiosink")
        return new AudioJackSink();
    else if (sink_ == "alsasink")
        return new AudioAlsaSink();
    else if (sink_ == "pulsesink")
        return new AudioPulseSink();
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


