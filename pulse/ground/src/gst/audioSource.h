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

#include "gstBase.h"
#include <gst/audio/multichannel.h>

class AudioConfig;

class AudioSource : public GstBase
{
public:
	static AudioSource* create(const AudioConfig &config);
	virtual ~AudioSource();
	virtual void init();
protected:
	AudioSource(const AudioConfig &config);
	virtual void linkElements();
	const AudioConfig &config_;
	std::vector < GstElement * >sources_, aconvs_, queues_;
	GstElement *interleave_;
private:
	friend class AudioSender;
	static const GstAudioChannelPosition VORBIS_CHANNEL_POSITIONS[][8];
	void init_interleave();
	void set_channel_layout();
};

class AudioTestSource : public AudioSource
{
public:
	virtual ~AudioTestSource(){
	}
	virtual void init();
	AudioTestSource(const AudioConfig &config) : AudioSource(config) {
	}
};

class AudioFileSource : public AudioSource
{
public:
	virtual ~AudioFileSource();
	virtual void init();
	AudioFileSource(const AudioConfig &config) : AudioSource(config) {
	}
private:
	virtual void linkElements();
	std::vector<GstElement *> decoders_;
};

class AudioAlsaSource : public AudioSource
{
public:
	virtual ~AudioAlsaSource(){
	}
	virtual void init();
	AudioAlsaSource(const AudioConfig &config) : AudioSource(config) {
	}
};

class AudioJackSource : public AudioSource
{
public:
	virtual ~AudioJackSource(){
	}
	virtual void init();
	AudioJackSource(const AudioConfig &config) : AudioSource(config) {
	}
};

#endif //_AUDIO_SOURCE_H_

