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
class AudioSourceConfig;

/** 
 *  \class AudioSource
 *  Abstract base class from which our audio sources are derived.
 *  Uses template method to define the initialization process that subclasses will have to
 *  implement (in part) and/or override. Any direct, concrete descendant of this class will not
 *  need to have its audio frames interleaved.
 */

class AudioSource
    : public GstLinkableSource
{
    public:
        /** Destructor */
        ~AudioSource();
        /** Object initializer */
        void init();

    protected:
        /** Constructor */
        explicit AudioSource(const AudioSourceConfig &config)
            : config_(config), sources_(0), aconvs_(0) {}
        /** 
         * Initialize source_/sources_ */
        virtual void init_source();
        /** 
         * Implemented by subclasses to perform other specific initialization */
        virtual void sub_init() = 0;
        /** 
        * Link pads of all our component GstElements */
        virtual void link_elements();
        /** 
         * Audio parameter object */
        const AudioSourceConfig &config_;
        /** 
         * GstElements representing each source and audioconvert */
        std::vector<GstElement *>sources_, aconvs_;
        /** 
         * Asynchronous callback that when triggered will call the appropriate callback in derived classes. */
        static gboolean base_callback(GstClock *clock, GstClockTime time, GstClockID id,
                                      gpointer user_data);
        /** 
         * Derived classes asynchronous callback. */
        virtual gboolean callback() { return FALSE; }
        /**
         * Returns this AudioSource's source, which is an audioconverter. */
        GstElement *srcElement() { return aconvs_[0]; }

    private:
        /**
         * No Copy Constructor */
        AudioSource(const AudioSource&);     
        /**
         * No Assignment Operator */
        AudioSource& operator=(const AudioSource&);     
};

/** 
 *  \class InterleavedAudioSource
 *  brief Abstract child of AudioSource whose audio frames must be interleaved before pushing audio further down pipeline.
 *
 */

class InterleavedAudioSource
    : public AudioSource
{
    public:
        /** Destructor */
        ~InterleavedAudioSource() {};

        /** Object initializer */
        void init();

    protected:
        /** Constructor */
        explicit InterleavedAudioSource(const AudioSourceConfig &config)
            : AudioSource(config), interleave_(config_) {}

        /** Overridden source initializer, which must initialize this object's Interleave object */
        void init_source();
        /** Links the pads of this InterleavedAudioSource's constituent elements */
        void link_elements();

        /** Object which performs the interleaving of this source's channels */
        Interleave interleave_;
        /** Exposes this InterleavedAudioSource's source, which is its Interleave's source */
        GstElement *srcElement() { return interleave_.srcElement(); }

    private:
        InterleavedAudioSource(const InterleavedAudioSource&);     //No Copy Constructor
        InterleavedAudioSource& operator=(const InterleavedAudioSource&);     //No Assignment Operator
};

/** 
 *  \class AudioTestSource
 *  Concrete InterleavedAudioSource which gives us an array of sine-wave generating sources.
 *
 *  AudioTestSource generates sine-tones, each of which alternate between two hard-coded frequencies. Their
 *  combined output is then interleaved.
 */

class AudioTestSource
    : public InterleavedAudioSource
{
    public:
        /** Constructor */
        explicit AudioTestSource(const AudioSourceConfig &config)
            : InterleavedAudioSource(config), clockId_(0), offset_(0) {}
        /** 
         * Destructor */
        ~AudioTestSource();
        
        void sub_init();

    protected:
        /** 
         * Asynchronous timed callback which will periodically toggle the 
         * frequency output by each channel */
        gboolean callback();

    private:
        void toggle_frequency();

        GstClockID clockId_;
        int offset_;

        static const double FREQUENCY[2][8];
        AudioTestSource(const AudioTestSource&);     //No Copy Constructor
        AudioTestSource& operator=(const AudioTestSource&);     //No Assignment Operator
};

/** 
 *  \class AudioFileSource
 *  Concrete AudioSource which provides playback of files. 
 *
 *  AudioFileSource plays back a file (determined by its AudioSourceConfig object). Depending
 *  on its AudioSourceConfig object, it may loop the file. It implements the BusMsgHandler interface
 *  so that it knows when an End-of-Signal event occurs, in which case it may
 *  play the file again if it has been set to continue to play the file (either infinitely or for a finite
 *  number of plays).
 */

class AudioFileSource
    : public AudioSource, public BusMsgHandler
{
    public:
        /** Constructor */
        explicit AudioFileSource(const AudioSourceConfig &config);
        /** 
         * Destructor */
        ~AudioFileSource();
        /** 
         * Handles EOS signal from bus, which may mean repeating playback of the file */
        bool handleBusMsg(_GstMessage *msg);
        /** 
         * Handles decoders' dynamically created pad(s) by linking them to the rest of the pipeline */
        static void cb_new_src_pad(GstElement * srcElement, GstPad * srcPad, gboolean last,
                                   void *data);
        
        void sub_init();
        /** 
         * AudioFileSource specific element linking method */
        void link_elements();

    private:
        void restartPlayback();
        std::vector<GstElement*> decoders_;
        AudioFileSource(const AudioFileSource&);     //No Copy Constructor
        AudioFileSource& operator=(const AudioFileSource&);     //No Assignment Operator
        int loopCount_;
};

// FIXME: create ABC FilteredAudioSource for alsa and pulse

/** 
 *  \class AudioAlsaSource
 *  Concrete AudioSource which captures audio from ALSA
 *
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioAlsaSource
    : public AudioSource
{
    public:
        /** Constructor */
        explicit AudioAlsaSource(const AudioSourceConfig &config)
            : AudioSource(config), capsFilter_(0) {}
        /** 
         * Destructor */
        ~AudioAlsaSource();

        void sub_init();
        /** Exposes this AudioAlsaSource's source, which is a capsfilter, as 
         * the number of channels must be explicitly set filtering the caps. */
        GstElement *srcElement() { return capsFilter_; }
    private:
        void link_elements();
        GstElement *capsFilter_;
        AudioAlsaSource(const AudioAlsaSource&);     //No Copy Constructor
        AudioAlsaSource& operator=(const AudioAlsaSource&);     //No Assignment Operator
};

/*! 
 *  \class AudioPulseSource
 *  Concrete AudioSource which captures audio from PulseAudio.
 *
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioPulseSource
    : public AudioSource
{
    public:
        /** Constructor */
        explicit AudioPulseSource(const AudioSourceConfig &config)
            : AudioSource(config), capsFilter_(0) {}
        /** 
         * Destructor */
        ~AudioPulseSource();
    
        void sub_init();
        /** 
         * Exposes this AudioAlsaSource's source, which is a capsfilter, as 
         * the number of channels must be explicitly set filtering the caps. */
        GstElement *srcElement() { return capsFilter_; }
    private:
        void link_elements();
        GstElement *capsFilter_;
        AudioPulseSource(const AudioPulseSource&);     //No Copy Constructor
        AudioPulseSource& operator=(const AudioPulseSource&);     //No Assignment Operator
};

/*! 
 *  \class AudioJackSource
 *  Concrete InterleavedAudioSource which captures audio from JACK.
 *  Interleaves incoming jack buffers into one multichannel stream.
 */


class AudioJackSource
    : public InterleavedAudioSource
{
    public:
        /** Constructor */
        explicit AudioJackSource(const AudioSourceConfig &config)
            : InterleavedAudioSource(config) {}
        void sub_init();
    private:
        AudioJackSource(const AudioJackSource&);     //No Copy Constructor
        AudioJackSource& operator=(const AudioJackSource&);     //No Assignment Operator
};

/*! 
 *  \class AudioDvSource
 *  Concrete AudioSource which captures audio from dv device.
 *
 *  This object is tightly coupled with VideoDvSource, as both (if present) will share one source_ GstElement, 
 *  and one dvdemux GstElement. It will look for both of these in the pipeline before trying to instantiate them. 
 *  If these GstElement are already present, AudioDvSource will simply store their addresses and link to them, if not
 *  it will create them. Audio can only be stereo or 4 channels according to gstreamer.
 */

class AudioDvSource
    : public AudioSource
{
    public:
        /** Constructor */
        explicit AudioDvSource(const AudioSourceConfig &config)
        : AudioSource(config), demux_(0), queue_(0), dvIsNew_(true) {}
        /** 
         * Destructor */
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


