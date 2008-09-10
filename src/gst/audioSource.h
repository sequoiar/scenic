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

#include "gstLinkable.h"
#include "interleave.h"

#include "audioDelaySource.h"

class AudioConfig;

class AudioSource
    : public GstLinkableSource
{
    public:
        ~AudioSource();
        bool init();

    protected:
        explicit AudioSource(const AudioConfig &config)
            : config_(config), sources_(0), aconvs_(0) {}

        virtual void init_source();
        virtual void sub_init() = 0;
        virtual void link_elements();

        const AudioConfig &config_;


        std::vector<GstElement *>sources_, aconvs_;
        static gboolean base_callback(GstClock *clock, GstClockTime time, GstClockID id,
                                      gpointer user_data);

        virtual gboolean callback() { return FALSE; }
        GstElement *srcElement() { return aconvs_[0]; }

    private:
        AudioSource(const AudioSource&);     //No Copy Constructor
        AudioSource& operator=(const AudioSource&);     //No Assignment Operator
        friend class AudioSender;
};

class InterleavedAudioSource
    : public AudioSource
{
    public:
        bool init();

        ~InterleavedAudioSource() {};

    protected:
        explicit InterleavedAudioSource(const AudioConfig &config)
            : AudioSource(config), interleave_(config_) {}

        void init_source();
        void link_elements();

        Interleave interleave_;
        GstElement *srcElement() { return interleave_.srcElement(); }

    private:
        InterleavedAudioSource(const InterleavedAudioSource&);     //No Copy Constructor
        InterleavedAudioSource& operator=(const InterleavedAudioSource&);     //No Assignment Operator
};


class AudioTestSource
    : public InterleavedAudioSource
{
    public:
        explicit AudioTestSource(const AudioConfig &config)
            : InterleavedAudioSource(config), clockId_(0), offset_(0) {}
        ~AudioTestSource();
        void sub_init();

    protected:
        gboolean callback();

    private:
        void toggle_frequency();

        GstClockID clockId_;
        int offset_;

        AudioTestSource(const AudioTestSource&);     //No Copy Constructor
        AudioTestSource& operator=(const AudioTestSource&);     //No Assignment Operator
};

class AudioFileSource
    : public AudioSource
{
    public:
        explicit AudioFileSource(const AudioConfig &config)
            : AudioSource(config), decoders_() {}
        ~AudioFileSource();
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, gboolean last,
                                   void *data);
    protected:
        void sub_init();
        void link_elements();

    private:
        std::vector<GstElement*> decoders_;
        AudioFileSource(const AudioFileSource&);     //No Copy Constructor
        AudioFileSource& operator=(const AudioFileSource&);     //No Assignment Operator

};


class AudioAlsaSource
    : public InterleavedAudioSource
{
    public:
        explicit AudioAlsaSource(const AudioConfig &config)
            : InterleavedAudioSource(config) {}
        void sub_init();
    private:
        AudioAlsaSource(const AudioAlsaSource&);     //No Copy Constructor
        AudioAlsaSource& operator=(const AudioAlsaSource&);     //No Assignment Operator
};


class AudioJackSource
    : public InterleavedAudioSource
{
    public:
        explicit AudioJackSource(const AudioConfig &config)
            : InterleavedAudioSource(config) {}
        void sub_init();
    private:
        AudioJackSource(const AudioJackSource&);     //No Copy Constructor
        AudioJackSource& operator=(const AudioJackSource&);     //No Assignment Operator
};


class AudioDvSource
    : public AudioSource
{
    public:
        explicit AudioDvSource(const AudioConfig &config)
        : AudioSource(config), demux_(0), queue_(0), dvIsNew_(true) {}

        ~AudioDvSource();
        void init_source();
        void sub_init();

    private:
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, void *data);
        void link_elements();
        GstElement *demux_, *queue_;
        bool dvIsNew_;
        AudioDvSource(const AudioDvSource&);     //No Copy Constructor
        AudioDvSource& operator=(const AudioDvSource&);     //No Assignment Operator
};

#endif //_AUDIO_SOURCE_H_


