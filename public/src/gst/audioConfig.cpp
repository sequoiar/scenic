
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

#include <string>
#include <iostream>
#include "audioConfig.h"
#include "audioSource.h"
#include "logWriter.h"

AudioConfig::AudioConfig(const std::string &source__,
    int numChannels__,
    const std::string &codec__,
    const std::string &remoteHost__,
    int port__)
    : MediaConfig(source__, codec__, remoteHost__, port__), numChannels_(numChannels__), location_("")
{
    // rtp sender
}


AudioConfig::AudioConfig(const std::string &source__,
    const std::string &location__,
    int numChannels__,
    const std::string &codec__,
    const std::string &remoteHost__,
    int port__)
    : MediaConfig(source__, codec__, remoteHost__,
                  port__), numChannels_(numChannels__), location_(location__)
{
    // rtp file sender
}


AudioConfig::AudioConfig(const std::string & source__, int numChannels__)
    : MediaConfig(source__), numChannels_(numChannels__), location_("")
{
    // local sender
}


AudioConfig::AudioConfig(const std::string & source__, const std::string & location__,
    int numChannels__)
    : MediaConfig(source__), numChannels_(numChannels__), location_(location__)
{
    // local file sender
}


AudioConfig::AudioConfig(int numChannels__, const std::string &codec__, int port__)
    : MediaConfig(codec__, port__), numChannels_(numChannels__), location_("")
{
    // receiver
}


// strips .delay from source name
const char *AudioConfig::source() const
{
    // find last period
    unsigned int pos = source_.rfind(".");

    if (pos < source_.size())
        return source_.substr(0, pos).c_str();
    else
        return MediaConfig::source();
}


const char* AudioConfig::location() const
{
    if (!location_.empty())
        return location_.c_str();
    else{
        LOG("No location specified", ERROR);
        return NULL;
    }
}


AudioSource* AudioConfig::createSource() const
{
    if (source_ == "audiotestsrc.delay")
        return new AudioDelaySource<AudioTestSource>(*this);
    else if (source_ == "filesrc.delay")
        return new AudioDelaySource<AudioFileSource>(*this);
    else if (source_ == "alsasrc.delay")
        return new AudioDelaySource<AudioAlsaSource>(*this);
    else if (source_ == "jackaudiosrc.delay")
        return new AudioDelaySource<AudioJackSource>(*this);
    else if (source_ == "audiotestsrc")
        return new AudioTestSource(*this);
    else if (source_ == "filesrc")
        return new AudioFileSource(*this);
    else if (source_ == "alsasrc")
        return new AudioAlsaSource(*this);
    else if (source_ == "jackaudiosrc")
        return new AudioJackSource(*this);
    else if (source_ == "dv1394src")
        return new AudioDvSource(*this);
    else {
        std::cerr << "Invalid source!" << std::endl;
        return 0;
    }
}


