
// videoReceiver.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include "media_base.h"
#include "rtp_receiver.h"

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

        std::tr1::shared_ptr<RtpPay> depayloader_;
        std::tr1::shared_ptr<VideoDecoder> decoder_;
        std::tr1::shared_ptr<TextOverlay> textoverlay_;
        std::tr1::shared_ptr<VideoScale> videoscale_;
        std::tr1::shared_ptr<VideoFlip> videoflip_;
        std::tr1::shared_ptr<VideoSink> sink_;
        bool gotCaps_;
};

#endif

