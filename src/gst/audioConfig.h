/* audioConfig.h
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _AUDIO_LOCAL_CONFIG_H_
#define _AUDIO_LOCAL_CONFIG_H_

#include <string>
#include "logWriter.h"

// forward declarations
class AudioSource;
class AudioSink;

/** Immutable class that is used to parameterize AudioLocal and AudioSender objects. */
class AudioSourceConfig
{
    public:
        /** Constructor sets by default location to an empty string and loop to LOOP_NONE */
        AudioSourceConfig(const std::string & source__, int numChannels__, int loop__ = LOOP_NONE)
            : source_(source__), location_(""), numChannels_(numChannels__), loop_(loop__)
        {
            if (source_.empty())
                THROW_ERROR("No source specified");
            if(numChannels_ < 1 || numChannels_ > 8)
                THROW_ERROR("Invalid number of channels");
        }
        /** 
         * Constuctor sets by default loop to LOOP_NONE, but has file location specified */
        AudioSourceConfig(const std::string & source__, const std::string & location__,
                int numChannels__, int loop__ = LOOP_NONE)
            : source_(source__), location_(location__), numChannels_(numChannels__) , loop_(loop__)
        {}
        /** 
         * Copy constructor */
        AudioSourceConfig(const AudioSourceConfig& m)
            : source_(m.source_), location_(m.location_), numChannels_(m.numChannels_) , loop_(m.loop_) 
        {}

        /** Returns c-style string specifying the source */
        const char *source() const;
        /** 
         * Returns number of channels */
        int numChannels() const { return numChannels_; }
        /** 
         * Returns number of times file will be played */
        int loop() const { return loop_; }
        /** 
         * Returns c-style string specifying the location 
         * (either filename or device descriptor) */
        const char *location() const;
        /** 
         * Returns true if location indicates an existing, readable file. */
        bool fileExists() const;
         
        /** Factory method that creates an AudioSource based on this object's source_ string */
        AudioSource* createSource() const;

        /** Enum representing two possible loop settings, any other will correspond 
         * to the finite number of times to playback. */
        enum LOOP_SETTING 
        { 
            LOOP_INFINITE = -1,
            LOOP_NONE = 0
        };
        
    private:
        /** No Assignment Operator */
        AudioSourceConfig& operator=(const AudioSourceConfig&); 
        const std::string source_;
        const std::string location_;
        const int numChannels_;
        const int loop_;
};

/**  Immutable class that is used to parametrize AudioReceiver objects.  */
class AudioSinkConfig 
{
    public:
        /** Constructor */
        AudioSinkConfig(const std::string & sink__)
            : sink_(sink__)
        {}
        /** 
         * Copy constructor */
        AudioSinkConfig(const AudioSinkConfig & m) : sink_(m.sink_) 
        {}

        /** Factory method that creates an AudioSink based on this object's sink_ string */
        AudioSink* createSink() const;

    private:
        const std::string sink_;
};

#endif // _AUDIO_LOCAL_CONFIG_H_

