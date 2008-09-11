
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

#ifndef _AUDIO_CONFIG_H_
#define _AUDIO_CONFIG_H_

#include <string>
#include "mediaConfig.h"

class AudioSource;
class AudioSink;

class AudioConfig
    : public MediaConfig
{
    public:
        // rtp sender
        AudioConfig(const std::string &source__,
                int numChannels__,
                const std::string &codec__,
                const std::string &remoteHost__,
                int port__)
            : MediaConfig(source__, codec__, remoteHost__, port__), numChannels_(numChannels__)
        {}


        // rtp file sender
        AudioConfig(const std::string &source__,
                const std::string &location__,
                int numChannels__,
                const std::string &codec__,
                const std::string &remoteHost__,
                int port__)
            : MediaConfig(source__, location__, codec__, remoteHost__,
                    port__), numChannels_(numChannels__)
        {}


        // local sender
        AudioConfig(const std::string & source__, int numChannels__)
            : MediaConfig(source__), numChannels_(numChannels__)
        {}


        // local file sender
        AudioConfig(const std::string & source__, const std::string & location__,
                int numChannels__)
            : MediaConfig(source__, location__), numChannels_(numChannels__)
        {}


        // receiver
        AudioConfig(int numChannels__, const std::string &codec__, int port__)
            : MediaConfig(codec__, port__), numChannels_(numChannels__)
        {}

        // copy constructor
        AudioConfig(const AudioConfig& m)
            : MediaConfig(m), numChannels_(m.numChannels_) {}

        const char *source() const;

        int numChannels() const { return numChannels_; };
        AudioSource* createSource() const;
        AudioSink* createSink() const;
        
        bool sanityCheck() const;

    private:

        AudioConfig& operator=(const AudioConfig&); //No Assignment Operator
        const int numChannels_;
};

#endif // _AUDIO_CONFIG_H_

