// audioLevel.h
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

#ifndef _AUDIO_LEVEL_H_
#define _AUDIO_LEVEL_H_

#include "gstLinkable.h"

class _GstElement;

class AudioLevel 
    : public GstLinkableFilter
{
    public:
        AudioLevel() : level_(0), rms_(0.0) {}

        ~AudioLevel();
        bool init();
        bool updateRms();

    protected:
        _GstElement *srcElement() { return level_; }
        _GstElement *sinkElement() { return level_; }

    private:
        void updateRms(double rmsDb);
        double dbToLinear(double db);

        _GstElement *level_;
        double rms_;

        AudioLevel(const AudioLevel&);     //No Copy Constructor
        AudioLevel& operator=(const AudioLevel&);     //No Assignment Operator
};

#endif //_AUDIO_LEVEL_H_

