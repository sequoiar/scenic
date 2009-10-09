/* audioConfig.cpp
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
#include "mapMsg.h"

#include "audioConfig.h"
#include "audioSource.h"
#include "audioSink.h"
#include "jackUtils.h"

AudioSourceConfig::AudioSourceConfig(MapMsg &msg) :
    source_(msg["source"]), 
    sourceName_(msg["jack-client-name"]),
    deviceName_(msg["device"]), 
    location_(msg["location"]), 
    numChannels_(msg["numchannels"])
{
    if (source_.empty())
        THROW_CRITICAL("No source specified");
    if(numChannels_ < 1)
        THROW_CRITICAL("Invalid number of channels");
    if (source_ == "jackaudiosrc")  // FIXME: this has to happen early but it's gross to have it here
        Jack::assertReady();
}


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
        THROW_CRITICAL(source_ << " is an invalid source");
    return 0;
}


/// Returns c-style string specifying the location (filename) 
const char* AudioSourceConfig::location() const
{
    return location_.c_str();
}


/// Returns c-style string specifying the source name
const char* AudioSourceConfig::sourceName() const
{
    if (sourceName_ != "")
        return sourceName_.c_str();
    else
        return 0;
}


/// Returns c-style string specifying the device (i.e. plughw:0)
const char* AudioSourceConfig::deviceName() const
{
    return deviceName_.c_str();
}


/// Returns true if location indicates an existing, readable file/device. 
bool AudioSourceConfig::locationExists() const
{
    return fileExists(location_);
}


/// Constructor 
AudioSinkConfig::AudioSinkConfig(MapMsg &msg) : 
    sink_(msg["sink"]), 
    sinkName_(msg["jack-client-name"]),
    deviceName_(msg["device"]), 
    bufferTime_(static_cast<int>(msg["audio-buffer-usec"]))
{
    if (sink_ == "jackaudiosink") // FIXME: it's good for this to happen early 
        Jack::assertReady();      // (before waiting on caps) but having it here is pretty gross
}

/// Factory method that creates an AudioSink based on this object's sink_ string 
AudioSink* AudioSinkConfig::createSink() const
{
    if (sink_ == "jackaudiosink")
        return new AudioJackSink(*this);
    else if (sink_ == "alsasink")
        return new AudioAlsaSink(*this);
    else if (sink_ == "pulsesink")
        return new AudioPulseSink(*this);
    else
    {
        THROW_CRITICAL(sink_ << " is an invalid sink");
        return 0;
    }
}


/// Returns c-style string specifying the source name
const char* AudioSinkConfig::sinkName() const
{
    if (sinkName_ != "")
        return sinkName_.c_str();
    else
        return 0;
}


/// Returns c-style string specifying the device used
const char* AudioSinkConfig::deviceName() const
{
    return deviceName_.c_str();
}


/// Returns buffer time, which must be an unsigned long long for gstreamer's audiosink to accept it safely
unsigned long long AudioSinkConfig::bufferTime() const
{
    return bufferTime_;
}

