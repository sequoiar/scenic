
// videoSender.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
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
#include "busMsgHandler.h"

class VideoSource;
class VideoEncoder;
class Payloader;
class _GstMessage;

class VideoSender
    : public SenderBase, public BusMsgHandler
{
    public:
        VideoSender(const VideoSourceConfig vConfig, const SenderConfig rConfig, bool capsOutOfBand);
        ~VideoSender();
        std::string getCaps() const;

    private:
        bool handleBusMsg(_GstMessage *msg);
        void init_source();
        void init_codec();
        void init_payloader();

        const VideoSourceConfig videoConfig_;
        SenderConfig remoteConfig_;
        RtpSender session_;
        VideoSource *source_;
        VideoEncoder *encoder_;
        Payloader *payloader_; 

        // hidden

        VideoSender(const VideoSender&); //No Copy Constructor
        VideoSender& operator=(const VideoSender&); //No Assignment Operator
};

#endif

