
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
#include "audioConfig.h"

AudioConfig::AudioConfig(std::string source, int numChannels, std::string codec, std::string remoteHost, int port) : MediaConfig(source, codec, remoteHost, port),
	numChannels_
	(numChannels)
{
	// empty
}

AudioConfig::AudioConfig(std::string source, int numChannels) : MediaConfig(source),
	numChannels_(numChannels)
{
	// empty
}

AudioConfig::AudioConfig(int numChannels, std::string codec, int port) : MediaConfig(codec, port),
	numChannels_(numChannels)
{
	// empty
}

const int AudioConfig::numChannels() const
{
	return numChannels_;
}

const bool AudioConfig::hasTestSrc() const
{
	return !source_.compare("audiotestsrc");
}

const bool AudioConfig::hasFileSrc() const
{
	return !source_.compare("filesrc");
}

const bool AudioConfig::hasAlsaSrc() const
{
	return !source_.compare("alsasrc");
}

const bool AudioConfig::hasJackSrc() const
{
	return !source_.compare("jackaudiosrc");
}
