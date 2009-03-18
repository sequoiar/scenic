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
#include "gstLinkable.h"

// forward declarations
class AudioSinkConfig;
class _GstElement;

/** Abstract base class representing a sink for audio streams */
class AudioSink : public GstLinkableSink
{
    public:
        AudioSink();
        
        ~AudioSink();
       
        std::string getCaps();

        virtual void init() = 0;

    protected:
        _GstElement *sink_;
        const static unsigned long long BUFFER_TIME;

    private:
        _GstElement *sinkElement() { return sink_; }

        /// No Copy Constructor 
        AudioSink(const AudioSink&);     
        /**
         * No Assignment Constructor */
        AudioSink& operator=(const AudioSink&);     
};

// FIXME: DRY!!! Either merge alsasink and pulsesink or pull out a common base class.

/// Concrete AudioSink class representing a sink to the ALSA interface 
class AudioAlsaSink : public AudioSink
{
    public:
        AudioAlsaSink(const AudioSinkConfig &config);
        
    private:
        ~AudioAlsaSink();
        
        void init();
        /** Returns this AudioAlsaSink's sink, which is an audioconverter, as 
         * raw-audio conversion happens before audio is output to ALSA */
        _GstElement *sinkElement() { return audioconvert_; }
        _GstElement *audioconvert_;

        const AudioSinkConfig &config_;

        /// No Copy Constructor
        AudioAlsaSink(const AudioAlsaSink&);     
        /// No Assignment Operator
        AudioAlsaSink& operator=(const AudioAlsaSink&);     
};

/// Concrete AudioSink class representing a sink to the Pulse interface 
class AudioPulseSink : public AudioSink
{
    public:
        AudioPulseSink(const AudioSinkConfig &config);
        ~AudioPulseSink();
    private:
        void init();
        _GstElement *sinkElement() { return audioconvert_; }
        _GstElement *audioconvert_;
        const AudioSinkConfig &config_;
        /// No Copy Constructor
        AudioPulseSink(const AudioPulseSink&);     
        /// No Assignment Operator
        AudioPulseSink& operator=(const AudioPulseSink&);     
};

/// Concrete AudioSink class representing a sink to the JACK audio connection kit 
class AudioJackSink : public AudioSink
{
    public:
        AudioJackSink();
        ~AudioJackSink();
    private:
        void init();
        /// No Copy Constructor 
        AudioJackSink(const AudioJackSink&);     
        /// No Assignment Operator 
        AudioJackSink& operator=(const AudioJackSink&);     
};
          

#endif //_AUDIO_SINK_H_

