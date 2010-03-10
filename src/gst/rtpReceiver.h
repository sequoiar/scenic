
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

class _GtkWidget;
class _GstRtpBin;
class _GstPad;

class RtpReceiver
    : RtpBin
{
    public:
        RtpReceiver(const Pipeline &pipeline) : 
            RtpBin(pipeline), 
            rtp_receiver_(0), 
            depayloader_(0), 
            recv_rtp_sink_(0), 
            send_rtcp_src_(0), 
            recv_rtcp_sink_(0) {}
        ~RtpReceiver();
        void setCaps(const char* capsStr);

        void add(RtpPay * depayloader, const ReceiverConfig & config);
        static void enableControl();
        static void updateLatencyCb(_GtkWidget *scale);
        static void setLatency(int latency);
        static const int MIN_LATENCY = 1; // ms
        static const int INIT_LATENCY = 50;   // ms
        static const int MAX_LATENCY = 5000; // ms

    private:
        virtual void subParseSourceStats(_GstStructure *stats);
        static _GstPad *getMatchingDepayloaderSinkPad(const std::string &srcMediaType);
        static std::string getMediaType(_GstPad *pad);
        static void onPadAdded(_GstElement * rtpbin, _GstPad * srcPad, void *data);
        static void onSenderTimeout(_GstElement * /* rtpbin */, unsigned /* session */, unsigned /* ssrc */, void * /*data*/);

        _GstElement *rtp_receiver_;
        _GstElement *depayloader_;
        _GstPad *recv_rtp_sink_;
        _GstPad *send_rtcp_src_;
        _GstPad *recv_rtcp_sink_;
        static std::list<_GstElement *> depayloaders_;
};

#endif // _RTP_RECEIVER_H_

