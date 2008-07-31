
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
#include "lo/lo.h"

#include "mediaBase.h"
#include "videoConfig.h"
#include "rtpSender.h"

class VideoSource;

class VideoSender
    : public MediaBase
{
    public:
        explicit VideoSender(const VideoConfig & config);
        bool start();
        void wait_for_stop();

        virtual ~VideoSender();

    private:
        virtual void init_source();
        virtual void init_codec();
        virtual void init_sink();

        static int stop_handler(const char *path, const char *types, lo_arg ** argv, int argc,
                                void *data,
                                void *user_data);

        static void liblo_error(int num, const char *msg, const char *path);

        const VideoConfig &config_;
        RtpSender session_;
        VideoSource *source_;
        GstElement *colorspc_, *encoder_, *payloader_, *sink_;

        // hidden

        VideoSender(const VideoSender&); //No Copy Constructor
        VideoSender& operator=(const VideoSender&); //No Assignment Operator
};

#endif

