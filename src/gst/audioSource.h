// audioSource.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _AUDIO_SOURCE_H_
#define _AUDIO_SOURCE_H_

#include <vector>
#include <string>
#include "busMsgHandler.h"

#include "noncopyable.h"

// forward declarations
class AudioSourceConfig;
class Pipeline;
class _GstElement;
class _GstMessage;

/**
 *  Abstract base class from which our audio sources are derived.
 *  Uses template method to define the initialization process that subclasses will have to
 *  implement (in part) and/or override.
 */

class AudioSource : private boost::noncopyable
{
    public:
        virtual ~AudioSource();
        virtual _GstElement *srcElement() { return source_; }

    protected:
        AudioSource(const Pipeline &pipeline, const AudioSourceConfig &config);

        const Pipeline &pipeline_;
        /// Audio parameter object
        const AudioSourceConfig &config_;

        _GstElement *source_;
};

/**
 *  Concrete AudioSource which gives us an array of sine-wave generating sources.
 *
 *  AudioTestSource generates sine-tones
 */

class AudioTestSource : public AudioSource
{
    public:
        AudioTestSource(const Pipeline &pipeline, const AudioSourceConfig &config);
    private:
        _GstElement *capsfilter_;
        _GstElement *srcElement();
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

class AudioFileSource : public AudioSource, private BusMsgHandler
{
    public:
        AudioFileSource(Pipeline &pipeline, const AudioSourceConfig &config);

    private:
        ~AudioFileSource();

        bool handleBusMsg(_GstMessage *msg);

        void loop(int nTimes);
        virtual _GstElement *srcElement() { return audioconvert_; }

        void restartPlayback();
        _GstElement *audioconvert_;
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
        virtual _GstElement *srcElement() { return capsfilter_; }

        _GstElement *capsfilter_;
        _GstElement *audioconvert_;
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
        virtual _GstElement *srcElement() { return capsfilter_; }

        _GstElement *capsfilter_;
        _GstElement *audioconvert_;
};

/**
 *  Concrete AudioSource which captures audio from JACK,
 *  and interleaves incoming jack buffers into one multichannel stream.
 */

class AudioJackSource : public AudioSource
{
    public:
        AudioJackSource(const Pipeline &pipeline, const AudioSourceConfig &config);
        void disableAutoConnect();

    private:
        virtual _GstElement *srcElement() { return queue_; }

        _GstElement *capsfilter_;
        _GstElement *queue_;
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

        _GstElement *queue_;
        _GstElement *audioconvert_;
        virtual _GstElement *srcElement() { return audioconvert_; }
};

#endif //_AUDIO_SOURCE_H_

