
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
#include <cassert>

#include "lo/lo.h"
#include "mediaBase.h"
#include "audioConfig.h"
#include "rtpReceiver.h"

class AudioReceiver
    : public MediaBase
{
    public:
        explicit AudioReceiver(const AudioConfig & config)
            : config_(config), session_(), gotCaps_(false), 
            depayloader_(0), decoder_(0), sink_(0)
        {}

        ~AudioReceiver()
        {
            assert(stop());
            pipeline_.remove(&sink_);
            pipeline_.remove(&decoder_);
            pipeline_.remove(&depayloader_);
        }

        bool start();

    private:
        AudioReceiver();
        void init_source(){};
        void init_codec();
        void init_sink();

        static int caps_handler(const char *path, const char *types, lo_arg ** argv, int argc,
                void *data,
                void *user_data);

        void set_caps(const char *caps);
        static void liblo_error(int num, const char *msg, const char *path);

        void wait_for_caps();

        const AudioConfig &config_;
        RtpReceiver session_;
        bool gotCaps_;
        GstElement *depayloader_, *decoder_, *sink_;

        AudioReceiver(const AudioReceiver&); //No Copy Constructor
        AudioReceiver& operator=(const AudioReceiver&); //No Assignment Operator
};

#endif // _AUDIO_RECEIVER_H_

