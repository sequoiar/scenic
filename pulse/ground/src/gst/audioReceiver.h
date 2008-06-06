
// audioReceiver.h
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

#ifndef _AUDIO_RECEIVER_H_
#define _AUDIO_RECEIVER_H_

#include <string>

#include "mediaBase.h"

class AudioReceiver : public MediaBase 
{
    public:
        AudioReceiver();
        virtual ~AudioReceiver();
        bool init(int port = DEF_PORT, int numChannels = 2);
        virtual bool start();

    private:
        int numChannels_;
        static const std::string CAPS_STR_2CH;
        static const std::string CAPS_STR_8CH;
};

#endif // _AUDIO_RECEIVER_H_

