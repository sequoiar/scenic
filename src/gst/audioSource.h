// audioSource.h
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

#ifndef _AUDIO_SOURCE_H_
#define _AUDIO_SOURCE_H_

#pragma GCC diagnostic ignored "-pedantic"
#include <gst/audio/multichannel.h>
#include <vector>
#include "gstLinkable.h"
#include "busMsgHandler.h"
#include "messageHandler.h"

#include "noncopyable.h"

// forward declarations
class AudioSourceConfig;
class Pipeline;

/** 
 *  Abstract base class from which our audio sources are derived.
 *  Uses template method to define the initialization process that subclasses will have to
 *  implement (in part) and/or override. Any direct, concrete descendant of this class will not
 *  need to have its audio frames interleaved.
 */

class AudioSource : public GstLinkableSource, boost::noncopyable
{
    public:
        ~AudioSource();

    protected:
        AudioSource(const Pipeline &pipeline, const AudioSourceConfig &config);
        
        const Pipeline &pipeline_;
        /// Audio parameter object 
        const AudioSourceConfig &config_;
        
        GstElement *source_;

        /// Caps used by any source with a capsfilter
        virtual std::string getCapsFilterCapsString();

        void initCapsFilter(GstElement* &aconv, GstElement* &capsfilter);

    private:
        
        GstElement *srcElement() { return source_; }
};

/** 
 * Abstract child of AudioSource whose audio frames must be interleaved before pushing audio further down pipeline.
 */

class InterleavedAudioSource : public AudioSource
{
    private:
        class Interleave : public GstLinkableFilter, boost::noncopyable
        {
            public:
                /** The interleave functionality used to be part of the same class
                 * as InterleavedAudioSource. When subdividing one class into two
                 * separate classes, it make sense for them to be friends. Also
                 * InterleavedAudioSource's internals are safe
                 * as InterleavedAudioSource's children will not have access here. */
                Interleave(const Pipeline &pipeline, const AudioSourceConfig &config);
                ~Interleave();

                GstElement *srcElement() { return interleave_; }
                GstElement *sinkElement() { return interleave_; }

            private:
                const Pipeline &pipeline_;
                GstElement *interleave_;
                const AudioSourceConfig &config_;
                static const GstAudioChannelPosition VORBIS_CHANNEL_POSITIONS[][8];
                void set_channel_layout();
        };

        GstElement *srcElement() { return interleave_.srcElement(); }

    protected:
        InterleavedAudioSource(const Pipeline &pipeline, const AudioSourceConfig &config);

        ~InterleavedAudioSource();

        /// Object which performs the interleaving of this source's channels 
        Interleave interleave_;
        std::vector<GstElement*> sources_, aconvs_;
};

/** 
 *  Concrete InterleavedAudioSource which gives us an array of sine-wave generating sources.
 *
 *  AudioTestSource generates sine-tones, each of which alternate between two hard-coded frequencies. Their
 *  combined output is then interleaved.
 */

class AudioTestSource : public InterleavedAudioSource
{
    public:
        AudioTestSource(const Pipeline &pipeline, const AudioSourceConfig &config);

    private:
        ~AudioTestSource();

        static int timedCallback(void *data);
        void toggle_frequency();

        std::vector< std::vector <double> > frequencies_;
        unsigned int callback_;
        int offset_;
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

class AudioFileSource : public AudioSource, BusMsgHandler
{
    public:
        AudioFileSource(Pipeline &pipeline, const AudioSourceConfig &config);

    private:
        ~AudioFileSource();

        bool handleBusMsg(GstMessage *msg);

        void loop(int nTimes);
        GstElement *srcElement() { return aconv_; }

        void restartPlayback();
        GstElement *aconv_;
        int loopCount_;
        static const int LOOP_INFINITE;
};

/** 
 *  Concrete AudioSource which captures audio from ALSA
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioAlsaSource : public AudioSource
{
    public:
        AudioAlsaSource(const Pipeline &pipeline, const AudioSourceConfig &config);

    private:
        ~AudioAlsaSource();

        GstElement *srcElement() { return capsFilter_; }

        GstElement *capsFilter_;
        GstElement *aconv_;
};

/** 
 *  Concrete AudioSource which captures audio from PulseAudio
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioPulseSource : public AudioSource
{
    public:
        AudioPulseSource(const Pipeline &pipeline, const AudioSourceConfig &config);
    private:
        ~AudioPulseSource();

        GstElement *srcElement() { return capsFilter_; }

        GstElement *capsFilter_;
        GstElement *aconv_;
};

/** 
 *  Concrete AudioSource which captures audio from JACK, 
 *  and interleaves incoming jack buffers into one multichannel stream.
 */

class AudioJackSource : public AudioSource, public MessageHandler
{
    public:
        AudioJackSource(const Pipeline &pipeline, const AudioSourceConfig &config);

    private:
        ~AudioJackSource();

        bool handleMessage(const std::string &path, const std::string &arguments);
        GstElement *srcElement() { return capsFilter_; }
        /// Caps used by any source with a capsfilter
        std::string getCapsFilterCapsString();

        GstElement *capsFilter_;
};


/** 
 *  Concrete AudioSource which captures audio from dv device.
 *
 *  This object is tightly coupled with VideoDvSource, as both (if present) will share one source_ GstElement, 
 *  and one dvdemux GstElement. It will look for both of these in the pipeline before trying to instantiate them. 
 *  If these GstElement are already present, AudioDvSource will simply store their addresses and link to them, if not
 *  it will create them. DVAudio can only be stereo or 4 channels according to gstreamer.
 */

class AudioDvSource : public AudioSource
{
    public:
        AudioDvSource(const Pipeline &pipeline, const AudioSourceConfig &config);

    private:
        ~AudioDvSource();

        GstElement *queue_;
        GstElement *aconv_;
        GstElement *srcElement() { return aconv_; }
};

#endif //_AUDIO_SOURCE_H_


