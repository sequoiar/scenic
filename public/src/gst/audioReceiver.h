
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

#include <cassert>
#include "mediaBase.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "rtpReceiver.h"
#include "audioLevel.h"
#include "logWriter.h"

class RtpPay;
class Decoder;
class AudioSink;

class AudioReceiver
    : public ReceiverBase
{
    public:
        AudioReceiver(const AudioReceiverConfig aConfig, const ReceiverConfig rConfig)
            : audioConfig_(aConfig), remoteConfig_(rConfig), session_(), gotCaps_(false), 
            depayloader_(0), decoder_(0), level_(), sink_(0)
        { assert(remoteConfig_.hasCodec()); }

        ~AudioReceiver();

        bool start();
        void set_caps(const std::string &caps) 
        { 
            session_.set_caps(caps.c_str()); 
            // FIXME: actually check caps
            if (!session_.ratesMatch())
                THROW_CRITICAL("Samplerates of receiver pipeline and incoming caps do not match");
            gotCaps_ = true;
        }

    private:
        AudioReceiver();
        void init_codec();
        void init_depayloader();
        void init_level();
        void init_sink();

        const AudioReceiverConfig audioConfig_;
        const ReceiverConfig remoteConfig_;

        RtpReceiver session_;
        bool gotCaps_;
        RtpPay *depayloader_;
        Decoder *decoder_;
        AudioLevel level_;
        AudioSink *sink_;

        AudioReceiver(const AudioReceiver&); //No Copy Constructor
        AudioReceiver& operator=(const AudioReceiver&); //No Assignment Operator
};

#endif // _AUDIO_RECEIVER_H_

