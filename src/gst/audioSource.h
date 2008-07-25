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

#include <cassert>
#include "gstBase.h"
#include "interleave.h"

class AudioConfig;

class AudioSource : public GstBase
{
    public:
        virtual ~AudioSource();
        void init();
        virtual void sub_init() = 0;
    protected:
        AudioSource(const AudioConfig &config);
        virtual void link_elements();
        virtual void link_interleave();

        const AudioConfig &config_;
        Interleave interleave_;
        std::vector<GstElement *>sources_, aconvs_;
        static gboolean base_callback(GstClock *clock, GstClockTime time, GstClockID id, gpointer user_data);
        virtual gboolean callback() { return FALSE; }

    private:
        friend class AudioSender;
};

class AudioTestSource : public AudioSource
{
    public:
        AudioTestSource(const AudioConfig &config) : AudioSource(config), clockId_(0), offset_(0) {}
        ~AudioTestSource();
        void sub_init();
    protected:
        virtual gboolean callback();

    private:
        void toggle_frequency();
        GstClockID clockId_;
        int offset_;

        AudioTestSource(const AudioTestSource&); //No Copy Constructor
        AudioTestSource& operator=(const AudioTestSource&); //No Assignment Operator
};

class AudioFileSource : public AudioSource
{
    public:
        AudioFileSource(const AudioConfig &config) : AudioSource(config), decoders_() {}
        ~AudioFileSource();
        void sub_init();
        void link_elements();
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, gboolean last, void *data);

    private:
        std::vector<GstElement*> decoders_;
};

class AudioAlsaSource : public AudioSource
{
    public:
        AudioAlsaSource(const AudioConfig &config) : AudioSource(config) {}
        void sub_init(){};
};

class AudioJackSource : public AudioSource
{
    public:
        AudioJackSource(const AudioConfig &config): AudioSource(config) {} 
        void sub_init();
};

template <typename T>
class AudioDelaySource : public T//, virtual public AudioSource
{
    public:
        void sub_init();
        void link_elements();
        void link_interleave();
        AudioDelaySource(const AudioConfig &config) : T(config), filters_() {} 
        ~AudioDelaySource();
        gboolean callback(GstClock *clock, GstClockTime time, GstClockID id);
    private:
        std::vector<GstElement *> filters_;
};

    template <typename T>
void AudioDelaySource<T>::sub_init()
{
    std::vector<GstElement*>::iterator iter;
    T::sub_init();

    for (int channelIdx = 0; channelIdx < T::config_.numChannels(); channelIdx++)
    {
        filters_.push_back(gst_element_factory_make("ladspa-delay-5s", NULL));
        assert(filters_[channelIdx]);
    }

    T::pipeline_.add_vector(filters_);

    for (iter = filters_.begin(); iter != filters_.end(); ++iter)
        g_object_set(G_OBJECT(*iter), "Delay", 0.5, NULL);
}

    template <typename T>
void AudioDelaySource<T>::link_elements()
{
    T::link_elements(); // link elements that precede the filters

    // FIXME: why can't I use GstIter typedef?
    std::vector<GstElement*>::iterator aconv, filter;

    for (aconv = T::aconvs_.begin(), filter = filters_.begin(); 
            aconv != T::aconvs_.end(), filter != filters_.end(); ++aconv, ++filter)
        gst_element_link(*aconv, *filter);
}

    template <typename T>
void AudioDelaySource<T>::link_interleave()
{
    std::vector<GstElement*>::iterator filter;
    for (filter = filters_.begin(); filter != filters_.end(); ++filter)
        T::interleave_.linkInput(*filter);
}

    template <typename T>
gboolean AudioDelaySource<T>::callback(GstClock *clock, GstClockTime time, GstClockID id)
{
    static double d_secs = 5;

    for (std::vector<GstElement*>::iterator iter = filters_.begin(); iter != filters_.end(); ++iter)
        g_object_set(G_OBJECT(*iter), "Delay", d_secs, NULL);

    d_secs = d_secs - 0.5;
    if (d_secs <= 0)
        d_secs = 5;

    return T::callback();
}

    template <typename T>
AudioDelaySource<T>::~AudioDelaySource<T>()
{
    assert(T::pipeline_.stop());
    T::pipeline_.remove_vector(filters_);
}

#endif //_AUDIO_SOURCE_H_

