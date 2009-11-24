
// videoReceiver.h
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

#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include "mediaBase.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "rtpReceiver.h"

#include <boost/shared_ptr.hpp>

class RtpPay;
class VideoDecoder;
class VideoScale;
class VideoSink;

class VideoReceiver
    : public ReceiverBase
{
    public:
        VideoReceiver(boost::shared_ptr<VideoSinkConfig> vConfig, 
                boost::shared_ptr<ReceiverConfig> rConfig);

        ~VideoReceiver();
        void toggleFullscreen();

    private:

        void init_codec();
        void init_depayloader();
        void init_sink();
        void setCaps();

        boost::shared_ptr<VideoSinkConfig> videoConfig_;
        boost::shared_ptr<ReceiverConfig> remoteConfig_;
        RtpReceiver session_;

        RtpPay *depayloader_; 
        VideoDecoder *decoder_;
        VideoScale *videoscale_;
        VideoSink *sink_;
        bool gotCaps_;
};

#endif

