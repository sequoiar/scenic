
/* audioLevel.h
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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

#include <gdk/gdktypes.h>
#include "gstLinkable.h"
#include "busMsgHandler.h"

// forward declarations
class Pipeline;
class _GstElement;
class _GstMessage;
class _GtkWidget;

/** 
 *  A filter that calculates and periodically reports 
 *  the rms value of each audio channel passing through it.
 */

class AudioLevel : public GstLinkableFilter, BusMsgHandler
{
    public:
        AudioLevel(Pipeline &pipeline, GdkNativeWindow socketID);
        ~AudioLevel();
        void interval(unsigned long long newInterval);

        bool handleBusMsg(_GstMessage *msg);

        void emitMessages(bool doEmit);

    private:
        static void setValue(gdouble value, _GtkWidget *vumeter);
        _GstElement *srcElement() { return level_; }

        _GstElement *sinkElement() { return level_; }

        //void updateRms(double rmsDb, size_t channelIdx);

        static double dbToLinear(double db);

        void print(const std::vector<double> &rmsValues) const;

        Pipeline &pipeline_;
        _GstElement *level_;
        bool emitMessages_;
        _GtkWidget *vumeter_;

        AudioLevel(const AudioLevel&);     //No Copy Constructor
        AudioLevel& operator=(const AudioLevel&);     //No Assignment Operator
};

#endif //_AUDIO_LEVEL_H_

