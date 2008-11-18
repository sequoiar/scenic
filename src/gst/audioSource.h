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

/** \file audioSource.h
 *      A family of classes representing different types/sources of audio input.
 */

#ifndef _AUDIO_SOURCE_H_
#define _AUDIO_SOURCE_H_

#include "gstLinkable.h"
#include "interleave.h"
#include "busMsgHandler.h"

// forward declarations
class AudioConfig;

/*! 
 *  \class AudioSource
 *  \brief Abstract base class from which our audio sources are derived.
 *
 *  Uses template method to define the initialization process that subclasses will have to
 *  implement (in part) and/or override. Any direct descendant of this class will already
 *  have its channels interleaved.
 */

class AudioSource
    : public GstLinkableSource
{
    public:
        //! Class destructor
        ~AudioSource();
        //! Object initializer
        void init();

    protected:
        //! Class constructor
        explicit AudioSource(const AudioConfig &config)
            : config_(config), sources_(0), aconvs_(0) {}

        //! Initialize source_/sources_
        virtual void init_source();
        //! Implemented by subclasses to perform other initialization
        virtual void sub_init() = 0;
        //! Link pads of all our component GstElements
        virtual void link_elements();
        //! Audio parameter object
        const AudioConfig &config_;
        //! GstElements representing each source and audioconvert
        std::vector<GstElement *>sources_, aconvs_;
        //! Asynchronous callback that when triggered will call the appropriate callback in derived classes.
        static gboolean base_callback(GstClock *clock, GstClockTime time, GstClockID id,
                                      gpointer user_data);
        //! Derived classes asynchronous callback.  
        virtual gboolean callback() { return FALSE; }
        GstElement *srcElement() { return aconvs_[0]; }

    private:
        //!No Copy Constructor
        AudioSource(const AudioSource&);     
        //!No Assignment Operator
        AudioSource& operator=(const AudioSource&);     
        friend class AudioSender;
};

/*! 
 *  \class InterleavedAudioSource
 *  \brief Abstract child of AudioSource which must be interleaved before pushing audio
 *
 */

class InterleavedAudioSource
    : public AudioSource
{
    public:
        //! Class destructor
        ~InterleavedAudioSource() {};

        //! Object initializer
        void init();

    protected:
        //! Class constructor
        explicit InterleavedAudioSource(const AudioConfig &config)
            : AudioSource(config), interleave_(config_) {}

        void init_source();
        void link_elements();

        //! Object which performs the interleaving of this source's channels
        Interleave interleave_;
        GstElement *srcElement() { return interleave_.srcElement(); }

    private:
        InterleavedAudioSource(const InterleavedAudioSource&);     //No Copy Constructor
        InterleavedAudioSource& operator=(const InterleavedAudioSource&);     //No Assignment Operator
};

/*! 
 *  \class AudioTestSource
 *  \brief Concrete InterleavedAudioSource which gives us an array of sine-wave generating sources.
 *
 *  AudioTestSource generates sine-tones, each of which alternate between two hard-coded frequencies. Their
 *  combined output is then interleaved.
 */

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

        static const double FREQUENCY[2][8];
        AudioTestSource(const AudioTestSource&);     //No Copy Constructor
        AudioTestSource& operator=(const AudioTestSource&);     //No Assignment Operator
};

/*! 
 *  \class AudioFileSource
 *  \brief Concrete AudioSource which provides playback of files. 
 *
 *  AudioFileSource playsback a file (determined by its AudioConfig object). Depending
 *  on its AudioConfig object, it may loop the file. It implements the BusMsgHandler interface
 *  so that it knows when an End-of-Signal event occurs, in which case it may
 *  play the file again if it has been set to continue to play the file (either infinitely or for a finite
 *  number of plays).
 */

class AudioFileSource
    : public AudioSource, public BusMsgHandler
{
    public:
        explicit AudioFileSource(const AudioConfig &config);
        ~AudioFileSource();
        bool handleBusMsg(_GstMessage *msg);
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, gboolean last,
                                   void *data);
    protected:
        void sub_init();
        void link_elements();

    private:
        void restartPlayback();
        std::vector<GstElement*> decoders_;
        AudioFileSource(const AudioFileSource&);     //No Copy Constructor
        AudioFileSource& operator=(const AudioFileSource&);     //No Assignment Operator
        int loopCount_;
};

// FIXME: create ABC FilteredAudioSource for alsa and pulse

/*! 
 *  \class AudioAlsaSource
 *  \brief Concrete AudioSource which captures audio from ALSA
 *
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioAlsaSource
    : public AudioSource
{
    public:
        explicit AudioAlsaSource(const AudioConfig &config)
            : AudioSource(config), capsFilter_(0) {}
        ~AudioAlsaSource();
        void sub_init();
        GstElement *srcElement() { return capsFilter_; }
    private:
        void link_elements();
        GstElement *capsFilter_;
        AudioAlsaSource(const AudioAlsaSource&);     //No Copy Constructor
        AudioAlsaSource& operator=(const AudioAlsaSource&);     //No Assignment Operator
};

/*! 
 *  \class AudioPulseSource
 *  \brief Concrete AudioSource which captures audio from PulseAudio.
 *
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioPulseSource
    : public AudioSource
{
    public:
        explicit AudioPulseSource(const AudioConfig &config)
            : AudioSource(config), capsFilter_(0) {}
        ~AudioPulseSource();
        void sub_init();
        GstElement *srcElement() { return capsFilter_; }
    private:
        void link_elements();
        GstElement *capsFilter_;
        AudioPulseSource(const AudioPulseSource&);     //No Copy Constructor
        AudioPulseSource& operator=(const AudioPulseSource&);     //No Assignment Operator
};

/*! 
 *  \class AudioJackSource
 *  \brief Concrete InterleavedAudioSource which captures audio from JACK.
 *
 *  Interleaves incoming jack buffers into one multichannel stream.
 */


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

/*! 
 *  \class AudioDvSource
 *  \brief Concrete AudioSource which captures audio from dv device.
 *
 *  This object is tightly coupled with VideoDvSource, as both (if present) will share one source_ GstElement, 
 *  and one dvdemux GstElement. It will look for both of these in the pipeline before trying to instantiate them. 
 *  If these GstElement are already present, AudioDvSource will simply store their addresses and link to them, if not
 *  it will create them.
 */

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


