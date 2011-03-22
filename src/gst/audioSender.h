
// audioLocal.h
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

#ifndef _AUDIO_SENDER_H_
#define _AUDIO_SENDER_H_

#include "mediaBase.h"
#include "rtpSender.h"
#include "busMsgHandler.h"
#include "noncopyable.h"

#include <tr1/memory>

class AudioSource;
class AudioSourceConfig;
class SenderConfig;
class AudioLevel;
class Pipeline;
class Encoder;
class Pay;
class _GstMessage;


class AudioSender
    : public SenderBase, private boost::noncopyable
{
    public:
        AudioSender(Pipeline &pipeline,
                const std::tr1::shared_ptr<AudioSourceConfig> &aConfig,
                const std::tr1::shared_ptr<SenderConfig> &rConfig);

        ~AudioSender();

    private:
        void createSource(Pipeline &pipeline);
        void createCodec(Pipeline &pipeline);
        void createPayloader();

        std::tr1::shared_ptr<AudioSourceConfig> audioConfig_;
        Pipeline &pipeline_;
        RtpSender session_;
        std::tr1::shared_ptr<AudioSource> source_;
        std::tr1::shared_ptr<AudioLevel> level_;

        std::tr1::shared_ptr<Encoder> encoder_;
        std::tr1::shared_ptr<Pay> payloader_;
};

#endif // _AUDIO_SENDER_H_

