
// rtpReceiver.h
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

#ifndef _RTP_RECEIVER_H_
#define _RTP_RECEIVER_H_

#include "config.h"
#include <list>
#include "rtpBin.h"

class ReceiverConfig;
class _GstElement;
class _GstPad;
class RtpPay;

#ifdef CONFIG_DEBUG_LOCAL
class _GtkAdjustment;
class _GtkWidget;
#endif

class RtpReceiver
    : public RtpBin
{
    public:
        RtpReceiver() : rtp_receiver_(0), depayloader_(0) {}
        ~RtpReceiver();
        void setCaps(const char* capsStr);
        static void setLatency(int latency);
        void checkSampleRate();

        void add(RtpPay * depayloader, const ReceiverConfig & config);

    private:
        static _GstPad *getMatchingDepayloaderSinkPad(_GstPad *srcPad);
        static std::string getMediaType(_GstPad *pad);
        static void cb_new_src_pad(_GstElement * element, _GstPad * srcPad, void *data);
        static void createLatencyControl();
        static void updateLatencyCb(_GtkAdjustment *adj);
        static const int MIN_LATENCY = 3;

        _GstElement *rtp_receiver_;
        _GstElement *depayloader_;
        static std::list<_GstElement *> depayloaders_;

#ifdef CONFIG_DEBUG_LOCAL
        static bool madeControl_;
        static _GtkWidget *control_;
#endif

        RtpReceiver(const RtpReceiver&); //No Copy Constructor
        RtpReceiver& operator=(const RtpReceiver&); //No Assignment Operator
};

#endif // _RTP_RECEIVER_H_

