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
#include "busMsgHandler.h"
#include <climits>

class _GstElement;
class _GstMessage;

class AudioLevel 
    : public GstLinkableFilter, public BusMsgHandler
{
    public:
        AudioLevel() : level_(0), emitMessages_(true), rmsValues_(0), interval_(ULONG_MAX * 0.25) {}

        ~AudioLevel();
        bool init();
        void interval(unsigned long long newInterval);
        bool handleBusMsg(_GstMessage *msg);
        void emitMessages(bool doEmit);

    protected:
        _GstElement *srcElement() { return level_; }
        _GstElement *sinkElement() { return level_; }

    private:
        void updateRms(double rmsDb, size_t channelIdx);
        static double dbToLinear(double db);
        void print() const;
        void post() const;

        _GstElement *level_;
        bool emitMessages_;
        std::vector<double> rmsValues_;
        unsigned long long interval_;

        AudioLevel(const AudioLevel&);     //No Copy Constructor
        AudioLevel& operator=(const AudioLevel&);     //No Assignment Operator
};

#endif //_AUDIO_LEVEL_H_

