
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

class AudioConfig : public MediaConfig
{
    public:
        AudioConfig(std::string source, int numChannels, std::string codec, 
                std::string remoteHost, int port); // sender
        AudioConfig(int numChannels, std::string codec, int port); // receiver
        AudioConfig(std::string source, int numChannels); // local sender
        
        const int numChannels() const;
        
        // to be replaced
        const bool hasTestSrc() const;
        const bool hasFileSrc() const;
        const bool hasAlsaSrc() const;
        const bool hasJackSrc() const;

    protected: 
        const int numChannels_;
};

#endif // _AUDIO_CONFIG_H_

