// audioSource.h
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

#ifndef _AUDIO_SOURCE_H_
#define _AUDIO_SOURCE_H_

#include <gst/audio/multichannel.h>
#include "gstBase.h"
#include "interleave.h"

class AudioConfig;

class AudioSource : public GstBase
{
    public:
        virtual ~AudioSource();
        void init();
        virtual void sub_init(){}
    protected:
        AudioSource(const AudioConfig &config);
        virtual void linkElements();
        const AudioConfig &config_;
        std::vector<GstElement *>sources_, aconvs_, queues_;
        Interleave interleave_;

    private:
        friend class AudioSender;
};

class AudioTestSource : public AudioSource
{
    public:
        virtual ~AudioTestSource();
        virtual void sub_init();
        AudioTestSource(const AudioConfig &config) : AudioSource(config) {
        }

    private:
        GstClockID clockId_;
       void add_clock_callback();
       static gboolean callback(GstClock *clock, GstClockTime time, GstClockID id, gpointer user_data);
};

class AudioFileSource : public AudioSource
{
    public:
        virtual ~AudioFileSource();
        virtual void sub_init();
        AudioFileSource(const AudioConfig &config) : AudioSource(config) {
        }
    private:
        virtual void linkElements();
        std::vector<GstElement *> decoders_;
};

class AudioAlsaSource : public AudioSource
{
    public:
        AudioAlsaSource(const AudioConfig &config) : AudioSource(config) {
        }
};

class AudioJackSource : public AudioSource
{
    public:
        virtual void sub_init();
        AudioJackSource(const AudioConfig &config) : AudioSource(config) {
        }
};

template <class T>
class AudioDelaySource : public T
{
    public:

        virtual void sub_init();
        AudioDelaySource(const AudioConfig &config) : T(config) {
        }
};

template <class T>
void AudioDelaySource<T>::sub_init(){
    
    T::sub_init();

}

#endif //_AUDIO_SOURCE_H_

