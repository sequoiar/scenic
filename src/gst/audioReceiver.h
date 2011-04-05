/* audioReceiver.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _AUDIO_RECEIVER_H_
#define _AUDIO_RECEIVER_H_

#include "mediaBase.h"
#include "rtpReceiver.h"

#include "noncopyable.h"

#include <tr1/memory>

class Pipeline;
class RtpPay;
class Decoder;
class AudioLevel;
class AudioSink;
class AudioSinkConfig;
class ReceiverConfig;

/** 
 * An audio pipeline derived from ReceiverBase that receives audio over rtp,
 * decodes/converts it as needed and pushes it to its sink.
 */
class AudioReceiver
    : public ReceiverBase, boost::noncopyable
{
    public:
        AudioReceiver(Pipeline &pipeline,
                const std::tr1::shared_ptr<AudioSinkConfig> &aConfig,
                const std::tr1::shared_ptr<ReceiverConfig> &rConfig);

        ~AudioReceiver();

    private:
        void createCodec(Pipeline &pipeline);
        void createDepayloader();
        void createSink(Pipeline &pipeline);
        
        void setCaps(); 

        std::tr1::shared_ptr<AudioSinkConfig> audioConfig_;
        std::tr1::shared_ptr<ReceiverConfig> remoteConfig_;

        RtpReceiver session_;
        bool gotCaps_;
        std::tr1::shared_ptr<RtpPay> depayloader_;
        std::tr1::shared_ptr<Decoder> decoder_;
        std::tr1::shared_ptr<AudioLevel> level_;
        std::tr1::shared_ptr<AudioSink> sink_;
};

#endif // _AUDIO_RECEIVER_H_
