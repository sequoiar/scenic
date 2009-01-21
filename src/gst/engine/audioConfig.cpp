/* audioConfig.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
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

#include <fstream>
#include "audioConfig.h"
#include "audioSource.h"
#include "audioSink.h"

/// Constructor sets by default location to an empty string and loop to LOOP_NONE 
AudioSourceConfig::AudioSourceConfig(const std::string & source__, 
                  int numChannels__) : 
    source_(source__), location_(""), numChannels_(numChannels__)
{
    if (source_.empty())
        THROW_ERROR("No source specified");
    if(numChannels_ < 1 || numChannels_ > 8)
        THROW_ERROR("Invalid number of channels");
}


///  Constuctor sets by default loop to LOOP_NONE, but has file location specified 
AudioSourceConfig::AudioSourceConfig(const std::string & source__, 
                  const std::string & location__,
                  int numChannels__) : 
    source_(source__), location_(location__), numChannels_(numChannels__)
{}


/// Copy constructor 
AudioSourceConfig::AudioSourceConfig(const AudioSourceConfig& m) : 
    source_(m.source_), location_(m.location_), numChannels_(m.numChannels_)
{}


/// Returns c-style string specifying the source 
const char *AudioSourceConfig::source() const
{
    return source_.c_str();
}

/// Returns number of channels 
int AudioSourceConfig::numChannels() const 
{ 
    return numChannels_; 
}

/// Factory method that creates an AudioSource based on this object's source_ string 
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


/// Returns c-style string specifying the location (either filename or device descriptor) 
const char* AudioSourceConfig::location() const
{
    if (location_.empty())
        THROW_ERROR("No location specified");

    return location_.c_str();
}


/// Returns true if location indicates an existing, readable file/device. 
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

/// Constructor 
AudioSinkConfig::AudioSinkConfig(const std::string & sink__) : 
    sink_(sink__)
{}

/// Copy constructor 
AudioSinkConfig::AudioSinkConfig(const AudioSinkConfig & m) : sink_(m.sink_) 
{}

/// Factory method that creates an AudioSink based on this object's sink_ string 
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


