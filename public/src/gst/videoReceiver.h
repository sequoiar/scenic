
// videoReceiver.h
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

#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include "mediaBase.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "rtpReceiver.h"

class RtpPay;
class Decoder;
class VideoSink;

class VideoReceiver
    : public ReceiverBase
{
    public:
        VideoReceiver(const VideoReceiverConfig vConfig, const ReceiverConfig rConfig)
            : videoConfig_(vConfig), remoteConfig_(rConfig), session_(), depayloader_(0), 
            decoder_(0), sink_(0) {}

        void start();
        void stop();
        VideoSink *getVideoSink() { return sink_; }

        ~VideoReceiver();

    private:

        void init_codec();
        void init_depayloader();
        void init_sink();

        const VideoReceiverConfig videoConfig_;
        const ReceiverConfig remoteConfig_;
        RtpReceiver session_;

        RtpPay *depayloader_; 
        Decoder *decoder_;
        VideoSink *sink_;

        VideoReceiver(const VideoReceiver&); //No Copy Constructor
        VideoReceiver& operator=(const VideoReceiver&); //No Assignment Operator
};

#endif

