
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

//class AudioSource;

class AudioSender
    : public MediaBase
{
public:
    AudioSender(const AudioConfig & config);
    AudioSender();
    virtual ~AudioSender();
    virtual bool start();

private:
// helper methods

    virtual void init_source();
    virtual void init_codec();
    virtual void init_sink();

    void send_caps() const;
    const char *caps_str() const;

// data
    const AudioConfig &config_;
    RtpSender session_;
    AudioSource *source_;

    GstElement *encoder_;
    GstElement *payloader_;
    GstElement *sink_;

    AudioSender(const AudioSender&);     //No Copy Constructor
    AudioSender& operator=(const AudioSender&);     //No Assignment Operator
};

#endif // _AUDIO_SENDER_H_

