
// mediaBase.h
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

#ifndef _MEDIA_BASE_H_
#define _MEDIA_BASE_H_

#include <vector>
class MediaBase
{
    public:
        virtual bool start();
        virtual bool stop();
        bool isPlaying() const;
        virtual bool init();


    protected:

        typedef struct _GstElement GstElement;
        typedef struct _GstPad GstPad;
        typedef std::vector<GstElement*>::iterator GstIter;

        // call back to attach new src pad
        static void cb_new_src_pad(GstElement *element, GstPad *srcPad, void *data);

        MediaBase();
        virtual ~MediaBase(); 
        virtual void init_pipeline();
        virtual void init_source() = 0;
        virtual void init_codec() = 0;
        virtual void init_sink() = 0;
        void wait_until_playing();

        GstElement *pipeline_, *rtpbin_;

    private:

        bool verbose_;
        void make_verbose();
        static bool gstInitialized_;
};

#endif // _MEDIA_BASE_H_
