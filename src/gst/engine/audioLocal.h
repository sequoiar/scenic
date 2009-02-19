
// audioLocal.h
// Copyright (C) 2009 Société des arts technologiques (SAT)
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

#ifndef _AUDIO_LOCAL_H_
#define _AUDIO_LOCAL_H_

#include "mediaBase.h"
#include "audioConfig.h"
#include "audioLevel.h"

// forward declarations
class AudioSource;
class AudioSink;

/** 
*   An audio pipeline derived from LocalBase whose source and sink are in the 
*   same process. */

class AudioLocal
    : public LocalBase 
{
    public:
        AudioLocal(const AudioSourceConfig srcConfig, const AudioSinkConfig sinkConfig);
        
        ~AudioLocal();

    private:
        void init_source();
        void init_level();
        void init_sink();

        // data
        const AudioSourceConfig srcConfig_;
        const AudioSinkConfig sinkConfig_;
        AudioSource *source_;
        AudioLevel level_;
        AudioSink *sink_;

        /// No Copy Constructor
        AudioLocal(const AudioLocal&); 
        /// No Assignment Operator
        AudioLocal& operator=(const AudioLocal&); 
};

#endif // _AUDIO_LOCAL_H_

