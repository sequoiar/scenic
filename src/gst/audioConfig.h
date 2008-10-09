
// audioConfig.h
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
 *      Immutable class that is used to setup AudioSender/AudioReceiver objects.
 *
 */

#ifndef _AUDIO_LOCAL_CONFIG_H_
#define _AUDIO_LOCAL_CONFIG_H_

#include <string>
#include "logWriter.h"

class AudioSource;
class AudioSink;

class AudioConfig
{
    public:

        // local 
        AudioConfig(const std::string & source__, int numChannels__)
            : source_(source__), location_(""), numChannels_(numChannels__)
        {
            if (source_.empty())
                THROW_ERROR("No source specified");
            if(numChannels_ < 1 || numChannels_ > 8)
                THROW_ERROR("Invalid number of channels");
        }


        // local file sender
        AudioConfig(const std::string & source__, const std::string & location__,
                int numChannels__)
            : source_(source__), location_(location__), numChannels_(numChannels__)
        {}

        // copy constructor
        AudioConfig(const AudioConfig& m)
            : source_(m.source_), location_(m.location_), numChannels_(m.numChannels_) {}

        const char *source() const;
        int numChannels() const { return numChannels_; };
        const char *location() const;
        bool fileExists() const;

        AudioSource* createSource() const;
        AudioSink* createSink() const;
        
    private:

        AudioConfig& operator=(const AudioConfig&); //No Assignment Operator
        const std::string source_;
        const std::string location_;
        const int numChannels_;
};

class AudioReceiverConfig 
{
    public:
        AudioReceiverConfig(const std::string & sink__)
            : sink_(sink__)
        {}

        // copy constructor
        AudioReceiverConfig(const AudioReceiverConfig & m) : sink_(m.sink_) 
        {}

        AudioSink* createSink() const;

    private:

        const std::string sink_;
};

#endif // _AUDIO_LOCAL_CONFIG_H_

