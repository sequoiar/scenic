
// videoSender.h
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

#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include "mediaBase.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "rtpSender.h"

class VideoSource;
class Encoder;
class RtpPay;

class VideoSender
    : public SenderBase 
{
    public:
         VideoSender(const VideoSourceConfig vConfig, const SenderConfig rConfig) 
            : videoConfig_(vConfig), remoteConfig_(rConfig), session_(), source_(0), 
            encoder_(0), payloader_(0) {}

        ~VideoSender();

        std::string getCaps() const;

    private:
        void init_source();
        void init_codec();
        void init_payloader();

        const VideoSourceConfig videoConfig_;
        const SenderConfig remoteConfig_;
        RtpSender session_;
        VideoSource *source_;
        Encoder *encoder_;
        RtpPay *payloader_; 

        // hidden

        VideoSender(const VideoSender&); //No Copy Constructor
        VideoSender& operator=(const VideoSender&); //No Assignment Operator
};

#endif

