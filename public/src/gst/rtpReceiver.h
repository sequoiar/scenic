
// rtpReceiver.h
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

#ifndef _RTP_RECEIVER_H_
#define _RTP_RECEIVER_H_

#include <list>
#include <gst/gst.h>
#include "rtpSession.h"

class MediaConfig;

class RtpReceiver
    : public RtpSession
{
    public:
        RtpReceiver();
        void set_caps(const char *capsStr);

        ~RtpReceiver();

    protected:
        void addDerived(GstElement * depayloader, const MediaConfig & config);

    private:
        static GstPad *get_matching_sink_pad(GstPad *srcPad);
        static void cb_new_src_pad(GstElement * element, GstPad * srcPad, void *data);

        GstElement *rtp_receiver_;
        GstElement *depayloader_;
        static std::list<GstElement *> depayloaders_;

        RtpReceiver(const RtpReceiver&); //No Copy Constructor
        RtpReceiver& operator=(const RtpReceiver&); //No Assignment Operator
};

#endif // _RTP_RECEIVER_H_

