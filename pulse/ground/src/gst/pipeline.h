
// pipline.h
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
        static Pipeline &Instance();
        void add(GstElement *element);
        void add_vector(std::vector <GstElement*> &elementVec);
        void remove(GstElement *element);
        void remove_vector(std::vector <GstElement*> &elementVec);
        bool isPlaying() const;
        void init();
        bool start();
        bool stop();
        
        // call back to attach new src pad
        static void cb_new_src_pad(GstElement *element, GstPad *srcPad, void *data);
        static void cb_new_sink_pad(GstElement *element, GstPad *sinkPad, void *data);

    private:
        Pipeline(); 
        ~Pipeline(); 
#if 0
        typedef struct _GstElement GstElement;
        typedef struct _GstPad GstPad;
        typedef std::vector<GstElement*>::iterator GstIter;
#endif
        static Pipeline* instance_;

        void make_verbose();
        GstElement *pipeline_;
        bool verbose_;
};

#endif // _PIPELINE_H_

