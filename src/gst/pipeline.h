
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
#include <gst/gst.h>

class Pipeline
{
    public:
        static Pipeline & Instance();
        void add(GstElement * element);
        void add(std::vector< GstElement * >&elementVec);

        GstClockID add_clock_callback(GstClockCallback callback, gpointer user_data);
        void remove_clock_callback(GstClockID clockId);

        void remove(GstElement ** element);
        void remove(std::vector < GstElement * >&elementVec);
        void reset();
        bool isPlaying() const;
        bool isPaused() const;
        void wait_until_playing() const;
        void wait_until_paused() const;
        void wait_until_stopped() const;
        bool start();
        bool stop();

        GstBus* getBus() const
        {
            return gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
        }


        GstClock* clock() const
        {
            return gst_pipeline_get_clock(GST_PIPELINE(pipeline_));
        }


        GstElement *findElement(const char *name)
        {
            return gst_bin_get_by_name(GST_BIN(pipeline_), name);
        }


    private:
        void init();
        static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);
        bool checkStateChange(GstStateChangeReturn ret);

        Pipeline(const Pipeline&);
        Pipeline& operator=(const Pipeline&);

        Pipeline();
        ~Pipeline();
        static Pipeline *instance_;

        void make_verbose();

        GstElement *pipeline_;
        GstClockTime startTime_;
        bool verbose_;
};

#endif // _PIPELINE_H_

