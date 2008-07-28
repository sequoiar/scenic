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

#include "audioDelaySource.h"

class AudioConfig;

class AudioSource
    : public GstBase
{
public:
    virtual ~AudioSource();
    void init();
    virtual void link_output(GstElement *sink);

protected:
    AudioSource(const AudioConfig &config);
    virtual void init_source();
    virtual void sub_init() = 0;
    virtual void link_elements();
    virtual void link_interleave();

    const AudioConfig &config_;
    Interleave interleave_;
    std::vector<GstElement *>sources_, aconvs_;
    static gboolean base_callback(GstClock *clock, GstClockTime time,GstClockID id,
                                  gpointer user_data);

    virtual gboolean callback() { return FALSE; }

private:
    friend class AudioSender;
};

class AudioTestSource
    : public AudioSource
{
public:
    AudioTestSource(const AudioConfig &config)
        : AudioSource(config), clockId_(0), offset_(0) {}
    ~AudioTestSource();
    void sub_init();

protected:
    virtual gboolean callback();

private:
    void toggle_frequency();

    GstClockID clockId_;
    int offset_;

    AudioTestSource(const AudioTestSource&);         //No Copy Constructor
    AudioTestSource& operator=(const AudioTestSource&);         //No Assignment Operator
};

class AudioFileSource
    : public AudioSource
{
public:
    AudioFileSource(const AudioConfig &config)
        : AudioSource(config), decoders_() {}
    ~AudioFileSource();
    void link_output(GstElement *sink);
    static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad,
                               gboolean last,
                               void *data);

protected:
    void sub_init();
    void link_elements();

    void link_interleave(){};            // FIXME: AudioFileSource shouldn't even have an
    // interleave, unless we support the option of
    void init_source();                  // multiple mono files combined into one stream

    std::vector<GstElement*> decoders_;
};

class AudioAlsaSource
    : public AudioSource
{
public:
    AudioAlsaSource(const AudioConfig &config)
        : AudioSource(config) {}
    void sub_init(){};
};

class AudioJackSource
    : public AudioSource
{
public:
    AudioJackSource(const AudioConfig &config)
        : AudioSource(config) {}
    void sub_init();
};


#endif //_AUDIO_SOURCE_H_

