
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
#include "rtpReceiver.h"

#include <tr1/memory>

class RtpPay;
class VideoDecoder;
class TextOverlay;
class VideoScale;
class VideoFlip;
class VideoSink;
class VideoSinkConfig;
class ReceiverConfig;

class VideoReceiver
    : public ReceiverBase
{
    public:
        VideoReceiver(Pipeline &pipeline,
                const std::tr1::shared_ptr<VideoSinkConfig> &vConfig,
                const std::tr1::shared_ptr<ReceiverConfig> &rConfig);

        ~VideoReceiver();
        void toggleFullscreen();

    private:
        void createCodec(Pipeline &pipeline);
        void createDepayloader();
        void createSink(Pipeline &pipeline);
        void setCaps();

        std::tr1::shared_ptr<VideoSinkConfig> videoConfig_;
        std::tr1::shared_ptr<ReceiverConfig> remoteConfig_;
        RtpReceiver session_;

        RtpPay *depayloader_; 
        VideoDecoder *decoder_;
        TextOverlay *textoverlay_;
        VideoScale *videoscale_;
        VideoFlip *videoflip_;
        VideoSink *sink_;
        bool gotCaps_;
};

#endif

