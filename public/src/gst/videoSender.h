
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
#include "rtpSender.h"

class VideoSource;
class VideoSink;

class VideoSender
    : public MediaBase
{
    public:
        explicit VideoSender(const VideoConfig config) 
            : config_(config), session_(), source_(0), colorspc_(0), 
            encoder_(0), payloader_(0), sink_(0) {}

        ~VideoSender();

        bool start();

    private:
        void init_source();
        void init_codec();
        void init_sink();

        const VideoConfig config_;
        RtpSender session_;
        VideoSource *source_;
        GstElement *colorspc_, *encoder_, *payloader_; 
        VideoSink *sink_;

        // hidden

        VideoSender(const VideoSender&); //No Copy Constructor
        VideoSender& operator=(const VideoSender&); //No Assignment Operator
};

#endif

