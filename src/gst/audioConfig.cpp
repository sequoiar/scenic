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
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "audioConfig.h"
#include "audioSource.h"
#include "audioLevel.h"
#include "audioSink.h"
#include "jackUtils.h"

static const int USEC_PER_MILLISEC = 1000;
namespace po = boost::program_options;

AudioSourceConfig::AudioSourceConfig(const po::variables_map &options) :
    source_(options["audiosource"].as<std::string>()), 
    bitrate_(options["audiobitrate"].as<int>()), 
    quality_(options["audioquality"].as<double>()), 
    sourceName_(options["jack-client-name"].as<std::string>()),
    deviceName_(options["audiodevice"].as<std::string>()), 
    location_(options["audiolocation"].as<std::string>()), 
    numChannels_(options["numchannels"].as<int>()),
    bufferTime_(options["audio-buffer"].as<int>() * USEC_PER_MILLISEC),
    socketID_(options["vumeter-id"].as<unsigned long>())
{
    using boost::lexical_cast;
    using std::string;
    if(numChannels_ < 1)
        throw std::range_error("Invalid number of channels=" + lexical_cast<string>(numChannels_));
}


int AudioSourceConfig::bitrate() const
{
    return bitrate_;
}


double AudioSourceConfig::quality() const
{
    return quality_;
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

/// Returns buffer time, which must be an unsigned long long for gstreamer's audiosink to accept it safely
unsigned long long AudioSourceConfig::bufferTime() const
{
    return bufferTime_;
}

/// Factory method that creates an AudioSource based on this object's source_ string 
AudioSource* AudioSourceConfig::createSource(Pipeline &pipeline) const
{
    if (source_ == "audiotestsrc")
        return new AudioTestSource(pipeline, *this);
    else if (source_ == "filesrc")
        return new AudioFileSource(pipeline, *this);
    else if (source_ == "alsasrc")
        return new AudioAlsaSource(pipeline, *this);
    else if (source_ == "jackaudiosrc") 
    {
        Jack::assertReady(pipeline);
        return new AudioJackSource(pipeline, *this);
    }
    else if (source_ == "dv1394src")
        return new AudioDvSource(pipeline, *this);
    else if (source_ == "pulsesrc")
        return new AudioPulseSource(pipeline, *this);
    else 
        THROW_ERROR(source_ << " is an invalid audiosource");
    return 0;
}

/// Factory method that creates an AudioLevel based on this object's socketID
AudioLevel* AudioSourceConfig::createLevel(Pipeline &pipeline) const
{
    if (socketID_ != 0)
        return new AudioLevel(pipeline, numChannels_, socketID_);
    else 
        return 0;
}

/// Fixme: abstract the common stuff into baseclass
/// Factory method that creates an AudioLevel based on this object's socketID
AudioLevel* AudioSinkConfig::createLevel(Pipeline &pipeline) const
{
    if (socketID_ != 0)
        return new AudioLevel(pipeline, numChannels_, socketID_);
    else 
        return 0;
}

/// Returns c-style string specifying the location (filename) 
const char* AudioSourceConfig::location() const
{
    return location_.c_str();
}


/// Returns c-style string specifying the device (i.e. plughw:0)
const char* AudioSourceConfig::deviceName() const
{
    return deviceName_.c_str();
}


/// Returns true if location indicates an existing, readable file/device. 
bool AudioSourceConfig::locationExists() const
{
    return boost::filesystem::exists(location_);
}


/// Constructor 
AudioSinkConfig::AudioSinkConfig(Pipeline &pipeline, const po::variables_map &options) : 
    sink_(options["audiosink"].as<std::string>()), 
    sinkName_(options["jack-client-name"].as<std::string>()),
    deviceName_(options["audiodevice"].as<std::string>()), 
    bufferTime_(options["audio-buffer"].as<int>() * USEC_PER_MILLISEC),
    socketID_(options["vumeter-id"].as<unsigned long>()),
    numChannels_(options["numchannels"].as<int>())
{ 
    // (before waiting on caps) but having it here is pretty gross
    if (sink_ == "jackaudiosink")
        Jack::assertReady(pipeline);
}

/// Factory method that creates an AudioSink based on this object's sink_ string 
AudioSink* AudioSinkConfig::createSink(Pipeline &pipeline) const
{
    if (sink_ == "jackaudiosink")
        return new AudioJackSink(pipeline, *this);
    else if (sink_ == "alsasink")
        return new AudioAlsaSink(pipeline, *this);
    else if (sink_ == "pulsesink")
        return new AudioPulseSink(pipeline, *this);
    else
    {
        THROW_CRITICAL(sink_ << " is an invalid audiosink");
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

int AudioSinkConfig::numChannels() const
{
    return numChannels_;
}

/// Returns buffer time, which must be an unsigned long long for gstreamer's audiosink to accept it safely
unsigned long long AudioSinkConfig::bufferTime() const
{
    return bufferTime_;
}

