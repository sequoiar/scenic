
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

#include "mediaBase.h"
#include "audioConfig.h"
#include "rtpSender.h"

class AudioSource;
class AudioSink;

class AudioSender
    : public MediaBase
{
    public:
        explicit AudioSender(const AudioConfig config) 
            : config_(config), session_(), source_(0), 
            encoder_(0), payloader_(0), sink_(0)
        {}

        ~AudioSender();

        std::string getCaps() { return session_.getCaps(); }
        bool start();

    private:
        // helper methods

        void init_source();
        void init_codec();
        void init_sink();

        void send_caps() const;

        // data
        const AudioConfig config_;
        RtpSender session_;
        AudioSource *source_;

        GstElement *encoder_;
        GstElement *payloader_;
        AudioSink *sink_;

        static const char* OSC_PORT;

        AudioSender(const AudioSender&); //No Copy Constructor
        AudioSender& operator=(const AudioSender&); //No Assignment Operator
};

#endif // _AUDIO_SENDER_H_

