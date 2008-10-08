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

class AudioSink 
    : public GstLinkableSink
{
    public:
        AudioSink()
            : sink_(0) {};
        ~AudioSink();
        std::string getCaps();

    protected:
        _GstElement *sinkElement() { return sink_; }
        _GstElement *sink_;

    private:
        AudioSink(const AudioSink&);     //No Copy Constructor
        AudioSink& operator=(const AudioSink&);     //No Assignment Operator
};

class AudioAlsaSink
: public AudioSink
{
    public:
        AudioAlsaSink() {};
        ~AudioAlsaSink() {};
        void init();
    private:
        AudioAlsaSink(const AudioAlsaSink&);     //No Copy Constructor
        AudioAlsaSink& operator=(const AudioAlsaSink&);     //No Assignment Operator
};


class AudioJackSink
: public AudioSink
{
    public:
        AudioJackSink() {};
        ~AudioJackSink() {};
        void init();
    private:
        AudioJackSink(const AudioJackSink&);     //No Copy Constructor
        AudioJackSink& operator=(const AudioJackSink&);     //No Assignment Operator
};
          

#endif //_AUDIO_SINK_H_

