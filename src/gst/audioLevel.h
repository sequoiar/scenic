/* audioLevel.h
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
#ifndef _AUDIO_LEVEL_H_
#define _AUDIO_LEVEL_H_

#include "gstLinkable.h"
#include "busMsgHandler.h"

// forward declarations
class _GstElement;
class _GstMessage;

/** AudioLevel
 *  A filter that calculates and periodically reports 
 *  the rms value of each audio channel passing through it.
 */

class AudioLevel 
    : public GstLinkableFilter, public BusMsgHandler
{
    public:
        /** Constructor sets by default emitMessages to true 
         * and message interval to one second */
        AudioLevel() : level_(0), emitMessages_(true), rmsValues_(0), interval_(1000000000LL) {}
        /** 
         * Destructor */
        ~AudioLevel();
        /** 
         * Class initializer */
        void init();
        /**
         * Sets the reporting interval in nanoseconds. */
        void interval(unsigned long long newInterval);
        /** 
         * The level message is posted on the bus by the level element, 
         * received by this AudioLevel, and dispatched. */
        bool handleBusMsg(_GstMessage *msg);
        /**
         * Toggles whether or not this AudioLevel will post messages on the bus. */
        void emitMessages(bool doEmit);

    private:
        /// Returns src of this AudioLevel. 
        _GstElement *srcElement() { return level_; }
        /**
         * Returns sink of this AudioLevel. */
        _GstElement *sinkElement() { return level_; }

        /// Updates most recent rms value of the specified channel. 
        void updateRms(double rmsDb, size_t channelIdx);
        /**
         * Converts from decibel to linear (0.0 to 1.0) scale. */
        static double dbToLinear(double db);
        /** 
         * Prints current rms values through the LogWriter system. */
        void print() const;
        /** 
         * Posts the rms values to be handled at a higher level by the MapMsg system. */
        void post() const;

        _GstElement *level_;
        bool emitMessages_;
        std::vector<double> rmsValues_;
        unsigned long long interval_;

        AudioLevel(const AudioLevel&);     //No Copy Constructor
        AudioLevel& operator=(const AudioLevel&);     //No Assignment Operator
};

#endif //_AUDIO_LEVEL_H_

