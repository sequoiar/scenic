
// audioLocal.h
// Copyright 2008 Tristan Matthews
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

#ifndef _AUDIO_LOCAL_H_
#define _AUDIO_LOCAL_H_

#include "mediaBase.h"
#include "audioConfig.h"
#include "audioLevel.h"

// forward declarations
class AudioSource;
class AudioSink;

/** \class AudioLocal
*   An audio pipeline derived from LocalBase whose source and sink are in the 
*   same process.
*/

class AudioLocal
    : public LocalBase 
{
    public:
        /// Constructor 
        explicit AudioLocal(const AudioSourceConfig srcConfig, const AudioSinkConfig sinkConfig) 
            : srcConfig_(srcConfig), sinkConfig_(sinkConfig), source_(0), level_(), sink_(0) {}
        /** 
         * Destructor */
        ~AudioLocal();

        //std::string getCaps();

    private:
        /** 
         * Implementation of LocalBase's template method which initializes this pipeline. */ 

        void init_source();
        void init_level();
        void init_sink();

        // data
        const AudioSourceConfig srcConfig_;
        const AudioSinkConfig sinkConfig_;
        AudioSource *source_;
        AudioLevel level_;
        AudioSink *sink_;

        AudioLocal(const AudioLocal&); //No Copy Constructor
        AudioLocal& operator=(const AudioLocal&); //No Assignment Operator
};

#endif // _AUDIO_LOCAL_H_

