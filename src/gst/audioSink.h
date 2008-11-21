// audioSink.h
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

#ifndef _AUDIO_SINK_H_
#define _AUDIO_SINK_H_
#include <string>
#include "gstLinkable.h"

class _GstElement;

/** \class AudioSink
 *  Abstract base class representing a sink for audio streams */
class AudioSink 
    : public GstLinkableSink
{
    public:
        /** Constructor */
        AudioSink()
            : sink_(0) {};
        /** Destructor */
        ~AudioSink();
        /**  Returns this AudioSink's caps */
        std::string getCaps();

    protected:
        _GstElement *sink_;

    private:
        /** Returns this AudioSink's sink */
        _GstElement *sinkElement() { return sink_; }

        /** No Copy Constructor */
        AudioSink(const AudioSink&);     
        /**
         * No Assignment Constructor */
        AudioSink& operator=(const AudioSink&);     
};

// FIXME: DRY!!! Either merge alsasink and pulsesink or pull out a common base class.

/** \class AudioAlsaSink
 *  Concrete AudioSink class representing a sink to the ALSA interface */
class AudioAlsaSink
: public AudioSink
{
    public:
        /** Constructor */
        AudioAlsaSink() : audioconvert_(0) {};
        /** 
         * Destructor */
        ~AudioAlsaSink();
        /** 
         * Object initialization method */
        void init();
    private:
        /** Returns this AudioAlsaSink's sink, which is an audioconverter, as 
         * raw-audio conversion happens before audio is output to ALSA */
        _GstElement *sinkElement() { return audioconvert_; }
        _GstElement *audioconvert_;
        AudioAlsaSink(const AudioAlsaSink&);     //No Copy Constructor
        AudioAlsaSink& operator=(const AudioAlsaSink&);     //No Assignment Operator
};

/** \class AudioPulseSink
 *  Concrete AudioSink class representing a sink to the Pulse interface */

class AudioPulseSink
: public AudioSink
{
    public:
        /** Constructor */
        AudioPulseSink() : audioconvert_(0) {};
        /**
         * Destructor */
        ~AudioPulseSink();
        /** 
         * Object initialization method */
        void init();
    private:
        _GstElement *sinkElement() { return audioconvert_; }
        _GstElement *audioconvert_;
        AudioPulseSink(const AudioPulseSink&);     //No Copy Constructor
        AudioPulseSink& operator=(const AudioPulseSink&);     //No Assignment Operator
};

/** \class AudioJackSink
 *  Concrete AudioSink class representing a sink to the JACK audio connection kit */

class AudioJackSink
: public AudioSink
{
    public:
        /** Constructor */
        AudioJackSink() {};
        /** 
         * Destructor */
        ~AudioJackSink() {};
        /** 
         * Object initialization method */
        void init();
    private:
        /** No Copy Constructor */
        AudioJackSink(const AudioJackSink&);     
        /** 
         * No Assignment Operator */
        AudioJackSink& operator=(const AudioJackSink&);     
};
          

#endif //_AUDIO_SINK_H_

