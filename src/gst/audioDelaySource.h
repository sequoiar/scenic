
// audioDelaySource.h
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

#ifndef _AUDIO_DELAY_SOURCE_H_
#define _AUDIO_DELAY_SOURCE_H_

#include <cassert>
#include "gstBase.h"

template <typename T>
class AudioDelaySource
    : public T
{
    typedef typename GstBase::GstIter GstIter;
public:
    void sub_init();
    void link_elements();
    void link_interleave();

    AudioDelaySource(const AudioConfig &config)
        : T(config), filters_() {}
    ~AudioDelaySource();
    gboolean callback(GstClock *clock, GstClockTime time, GstClockID id);

private:
    std::vector<GstElement *> filters_;
};

template <typename T>
void AudioDelaySource<T>::sub_init()
{
    GstIter iter;
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

    GstIter aconv, filter;

    for (aconv = T::aconvs_.begin(), filter = filters_.begin();
         aconv != T::aconvs_.end(), filter != filters_.end(); ++aconv, ++filter)
        gst_element_link(*aconv, *filter);
}

template <typename T>
void AudioDelaySource<T>::link_interleave()
{
    GstIter filter;
    for (filter = filters_.begin(); filter != filters_.end(); ++filter)
        T::interleave_.link_input(*filter);
}

template <typename T>
gboolean AudioDelaySource<T>::callback(GstClock *clock, GstClockTime time,
                                       GstClockID id)
{
    static double d_secs = 5;

    for (GstIter iter = filters_.begin(); iter != filters_.end(); ++iter)
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

