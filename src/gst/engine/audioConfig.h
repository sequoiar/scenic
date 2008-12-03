
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

// forward declarations
class AudioSource;
class AudioSink;

/// Immutable class that is used to parameterize AudioLocal and AudioSender objects. 
class AudioSourceConfig
{
    public:
        AudioSourceConfig(const std::string & source__, int numChannels__, int loop__ = LOOP_NONE);
        
        AudioSourceConfig(const std::string & source__, const std::string & location__,
                int numChannels__, int loop__ = LOOP_NONE);
        
        AudioSourceConfig(const AudioSourceConfig& m);

        const char *source() const;

        int numChannels() const;

        int loop() const;

        const char *location() const;

        bool fileExists() const;
         
        AudioSource* createSource() const;

        /** Enum representing two possible loop settings, any other will correspond 
         * to the finite number of times to playback. */
        enum LOOP_SETTING 
        { 
            LOOP_INFINITE = -1,
            LOOP_NONE = 0
        };
        
    private:
        /// No Assignment Operator 
        AudioSourceConfig& operator=(const AudioSourceConfig&); 
        const std::string source_;
        const std::string location_;
        const int numChannels_;
        const int loop_;
};

///  Immutable class that is used to parametrize AudioReceiver objects.  
class AudioSinkConfig 
{
    public:
        AudioSinkConfig(const std::string & sink__);
        
        AudioSinkConfig(const AudioSinkConfig & m); 

        AudioSink* createSink() const;

    private:
        const std::string sink_;
};

#endif // _AUDIO_LOCAL_CONFIG_H_

