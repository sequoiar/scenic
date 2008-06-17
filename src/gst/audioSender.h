
// audioSender.h
// Copyright 2008 Tristan Matthews 
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

#ifndef _AUDIO_SENDER_H_
#define _AUDIO_SENDER_H_

#include <string>
#include <gst/audio/multichannel.h>
#include "defaultAddresses.h"

#include "mediaBase.h"
#include "audioSession.h"

class AudioSender : public MediaBase
{
    public:
        AudioSender(const AudioSession& session);
        AudioSender();
        virtual ~AudioSender(); 
        bool init();
        const std::string caps_str() const;
        virtual bool start();

    private:

        // helper methods
       
        void set_channel_layout(GValueArray *arr);
        const bool isNetworked() const;

        // data 
        AudioSession session_;
        static const GstAudioChannelPosition VORBIS_CHANNEL_POSITIONS[][8];
};

#endif // _AUDIO_SENDER_H_

