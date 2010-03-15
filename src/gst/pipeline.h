
// pipeline.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
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

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include <vector>

#include "noncopyable.h"
#include <glib/gtypes.h>

class _GstElement;
class _GstPad;
class _GstBus;
class _GstMessage;
class BusMsgHandler;

class Pipeline : boost::noncopyable
{
    public:
        Pipeline();
        ~Pipeline();

        bool isAlive() { return pipeline_ == 0; }
        _GstElement *makeElement(const char *factoryName, const char *elementName) const;
        void subscribe(BusMsgHandler *obj);
        void unsubscribe(BusMsgHandler *obj);

        void updateSampleRate(unsigned newRate);
        void remove(_GstElement ** element) const;
        void remove(std::vector < _GstElement * >&elementVec) const;
        bool isPlaying() const;
        unsigned actualSampleRate() const;
        bool isReady() const;
        bool isPaused() const;
        bool isStopped() const;
        void seekTo(gint64 pos);
        void start() const;
        void pause() const;
        void makeReady() const;
        void stop() const;
        void quit() const;
        void makeVerbose() const;

    private:
        void add(_GstElement * element) const;
        _GstBus* getBus() const;

        static int bus_call(_GstBus *bus, _GstMessage *msg, void *data);

        void updateListeners(_GstMessage *msg);

        _GstElement *pipeline_;
        std::vector<BusMsgHandler*> handlers_;
        unsigned sampleRate_;
        char *titleStr_;
        static const unsigned int SAMPLE_RATE = 48000;
};

#endif // _PIPELINE_H_

