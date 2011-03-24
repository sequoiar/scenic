// audioSink.h
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

#ifndef _AUDIO_SINK_H_
#define _AUDIO_SINK_H_

#include <string>
#include "noncopyable.h"

// forward declarations
class AudioSinkConfig;
class Pipeline;
class _GstElement;

/** Abstract base class representing a sink for audio streams */
class AudioSink : private boost::noncopyable
{
    public:
        AudioSink(Pipeline &pipeline);
        virtual _GstElement *sinkElement() { return sink_; }
        
        virtual ~AudioSink();
       
        virtual void adjustBufferTime(unsigned long long);

    protected:
        Pipeline &pipeline_;
        _GstElement *sink_;
        const static unsigned long long BUFFER_TIME;

    private:
        static void FPE_ExceptionHandler(int nSig, int nErrType, int *pnReglist);
};

// FIXME: DRY!!! Either merge alsasink and pulsesink or pull out a common base class.

/// Concrete AudioSink class representing a sink to the ALSA interface 
class AudioAlsaSink : public AudioSink
{
    public:
        AudioAlsaSink(Pipeline &pipeline, const AudioSinkConfig &config);
        
    private:
        /** Returns this AudioAlsaSink's sink, which is an audioconverter, as 
         * raw-audio conversion happens before audio is output to ALSA */
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;

        const AudioSinkConfig &config_;
};

/// Concrete AudioSink class representing a sink to the Pulse interface 
class AudioPulseSink : public AudioSink
{
    public:
        AudioPulseSink(Pipeline &pipeline, const AudioSinkConfig &config);
    private:
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *aconv_;
        const AudioSinkConfig &config_;
};

/// Concrete AudioSink class representing a sink to the JACK audio connection kit
class AudioJackSink : public AudioSink
{
    public:
        AudioJackSink(Pipeline &pipeline, const AudioSinkConfig &config);
        void disableAutoConnect();
    private:
        const AudioSinkConfig &config_;
};
#endif //_AUDIO_SINK_H_

