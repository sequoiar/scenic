
// videoSender.h
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


#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include <string>
#include "defaultAddresses.h"

#include "mediaBase.h"
#include "videoConfig.h" 

class VideoSender : public MediaBase
{
    public:
        VideoSender(const VideoConfig& config);
        virtual ~VideoSender(); 
        virtual bool start();

    private:
        static void cb_new_src_pad(GstElement *element, GstPad *srcPad, void *data);
        virtual void init_source();
        virtual void init_codec();
        virtual void init_sink();
        VideoConfig config_;
        GstElement *source_, *demux_, *queue_, *dvdec_, *colorspc_, *encoder_, *payloader_, *sink_;
        GstElement *lastLinked_;     // FIXME: hack
};

#endif

