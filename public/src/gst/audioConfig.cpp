
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

AudioConfig::AudioConfig(const std::string &source, int numChannels, const std::string &codec, const std::string &remoteHost,
                         int port)
    : MediaConfig(source, codec, remoteHost, port), numChannels_(numChannels)
{
    // empty
}

AudioConfig::AudioConfig(const std::string & source, int numChannels) : MediaConfig(source),
    numChannels_(numChannels)
{
    // empty
}

AudioConfig::AudioConfig(int numChannels, const std::string &codec, int port) : MediaConfig(codec, port),
    numChannels_(numChannels)
{
    // empty
}

const int AudioConfig::numChannels() const
{
    return numChannels_;
}

// strips .delay from source name
const char *AudioConfig::source() const
{ 
    unsigned int pos = source_.find("."); 

    if (pos < source_.size()) 
    {
        std::string result(source_);
        return result.erase(pos, strlen(".delay")).c_str();
    }
    else
        return MediaConfig::source();
}


AudioSource* AudioConfig::createSource() const
{
    std::string s(source_);

    if (!s.compare("audiotestsrc.delay"))
        return new AudioDelaySource<AudioTestSource>(*this);
    else if (!s.compare("audiotestsrc"))
        return new AudioTestSource(*this);
    else if (!s.compare("filesrc"))
        return new AudioFileSource(*this);
    else if (!s.compare("alsasrc"))
        return new AudioAlsaSource(*this);
    else if (!s.compare("jackaudiosrc"))
        return new AudioJackSource(*this);
    else
    {
        std::cerr << "Invalid source!" << std::endl;
        return 0;
    }
}

