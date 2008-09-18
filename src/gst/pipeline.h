
// pipeline.h
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

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include <vector>
#include <gst/gstclock.h>
#include <gst/gstelement.h>

class _GstElement;
class _GstPad;
class _GstBus;
class _GstMessage;

class Pipeline
{
    public:
        static Pipeline & Instance();
        void add(_GstElement * element);
        void add(std::vector< _GstElement * >&elementVec);

        GstClockID add_clock_callback(GstClockCallback callback, void *user_data);
        void remove_clock_callback(GstClockID clockId);

        void remove(_GstElement ** element);
        void remove(std::vector < _GstElement * >&elementVec);
        void reset();
        bool isPlaying() const;
        bool isPaused() const;
        void wait_until_playing() const;
        void wait_until_paused() const;
        void wait_until_stopped() const;
        bool start();
        bool stop();

        _GstBus* getBus() const;

        GstClock* clock() const;

        _GstElement *findElement(const char *name) const;
        _GstPad *findUnconnectedSrcpad() const;
        _GstPad *findUnconnectedSinkpad() const;

    private:
        bool init();
        static gboolean bus_call(_GstBus *bus, _GstMessage *msg, void *data);
        bool checkStateChange(GstStateChangeReturn ret) const;

        Pipeline(const Pipeline&);
        Pipeline& operator=(const Pipeline&);

        Pipeline() : pipeline_(0), startTime_(0), verbose_(false) {}
        ~Pipeline();
        static Pipeline *instance_;

        void make_verbose();

        _GstElement *pipeline_;
        GstClockTime startTime_;
        bool verbose_;
};

#endif // _PIPELINE_H_

