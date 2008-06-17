
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
#include "audioConfig.h"

class AudioReceiver : public MediaBase 
{
    public:
        AudioReceiver();
        AudioReceiver(const AudioConfig& config);
        virtual ~AudioReceiver();
        bool init();
        //bool init(int port = DEF_PORT, int numChannels = 2);
        //bool init_uncomp(int port = DEF_PORT, int numChannels = 1);
        virtual bool start();

    private:
//        int numChannels_;
        AudioConfig config_;
        static const std::string CAPS_STR[2];
};

#endif // _AUDIO_RECEIVER_H_

